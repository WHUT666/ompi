#!/usr/bin/env python3
"""Generate a linkable PMIx stub library source from the public PMIx headers.

Scans pmix*.h for "PMIX_EXPORT <ret> PMIx_<name>(<args>);" declarations and
emits trivial definitions: pmix_status_t returns PMIX_ERR_NOT_SUPPORTED,
pointers return NULL (const char* returns a static string), everything else
returns 0.  This is Phase-3 scaffolding -- the real OpenPMIx build replaces
the generated file wholesale.

Usage: gen_pmix_stub.py <out.c> <include_dir> [include_dir ...]
"""

import re
import sys


def collect_decls(text, acc):
    # strip comments first (PMIX_EXPORT may appear in doc comments)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    for m in re.finditer(r"PMIX_EXPORT\b", text):
        # accumulate until a ';' at paren depth 0 (or '{' -> skip)
        i = m.end()
        depth = 0
        while i < len(text):
            c = text[i]
            if c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            elif c == ";" and depth == 0:
                break
            elif c == "{" and depth == 0:
                i = None
                break
            i += 1
        if i is None:
            continue
        stmt = text[m.end():i].strip()
        # need a function decl: must contain '(' before ';'
        if "(" not in stmt:
            continue
        # name = last identifier before first '('
        head = stmt[: stmt.index("(")]
        nm = re.search(r"([A-Za-z_][A-Za-z_0-9]*)\s*$", head)
        if not nm:
            continue
        name = nm.group(1)
        if not name.startswith(("PMIx_", "PMI_")):
            continue
        ret = head[: nm.start()].strip()
        args = stmt[stmt.index("("):]
        acc[name] = (ret, args)


# Hand-written bodies replacing the auto-generated NOT_SUPPORTED stubs for
# the functions a singleton MPI job needs.  PMIx_Init reports
# PMIX_ERR_UNREACH so Open MPI takes its singleton path (all subsequent
# modex ops become optional); a tiny local key/value store backs
# Put/Get so the process can resolve its own published data.
STUB_CORE = r"""
/* ------------------------------------------------------------------ */
/* Singleton support core (hand-written)                               */
/* ------------------------------------------------------------------ */

#define STUB_MAX_KV 4096

typedef struct {
    char nspace[PMIX_MAX_NSLEN + 1];
    pmix_rank_t rank;
    char key[PMIX_MAX_KEYLEN + 1];
    pmix_value_t val;
} stub_kv_t;

static stub_kv_t stub_kv[STUB_MAX_KV];
static int stub_nkv = 0;
static pmix_proc_t stub_self = { "pmix.stub", 0 };
static int stub_have_self = 0;

static void stub_val_copy(pmix_value_t *dst, const pmix_value_t *src)
{
    memcpy(dst, src, sizeof(*dst));
    switch (src->type) {
    case PMIX_STRING:
        dst->data.string = src->data.string ? strdup(src->data.string) : NULL;
        break;
    case PMIX_BYTE_OBJECT:
        dst->data.bo.bytes = NULL;
        dst->data.bo.size = 0;
        if (NULL != src->data.bo.bytes && 0 != src->data.bo.size) {
            dst->data.bo.bytes = malloc(src->data.bo.size);
            if (NULL != dst->data.bo.bytes) {
                memcpy(dst->data.bo.bytes, src->data.bo.bytes, src->data.bo.size);
                dst->data.bo.size = src->data.bo.size;
            }
        }
        break;
    case PMIX_PROC:
        dst->data.proc = NULL;
        if (NULL != src->data.proc) {
            dst->data.proc = malloc(sizeof(pmix_proc_t));
            if (NULL != dst->data.proc) {
                *dst->data.proc = *src->data.proc;
            }
        }
        break;
    case PMIX_ENVAR:
        dst->data.envar.envar = NULL;
        dst->data.envar.value = NULL;
        if (NULL != src->data.envar.envar) {
            dst->data.envar.envar = strdup(src->data.envar.envar);
        }
        if (NULL != src->data.envar.value) {
            dst->data.envar.value = strdup(src->data.envar.value);
        }
        break;
    default:
        /* scalar payload: struct copy already done */
        break;
    }
}

static void stub_val_free(pmix_value_t *v)
{
    if (NULL == v) {
        return;
    }
    switch (v->type) {
    case PMIX_STRING:
        free(v->data.string);
        break;
    case PMIX_BYTE_OBJECT:
        free(v->data.bo.bytes);
        break;
    case PMIX_PROC:
        free(v->data.proc);
        break;
    case PMIX_ENVAR:
        free(v->data.envar.envar);
        free(v->data.envar.value);
        break;
    default:
        break;
    }
    free(v);
}

static pmix_status_t stub_put(const pmix_proc_t *proc, const char *key,
                              const pmix_value_t *val)
{
    stub_kv_t *e;
    if (stub_nkv >= STUB_MAX_KV) {
        return PMIX_ERR_OUT_OF_RESOURCE;
    }
    e = &stub_kv[stub_nkv++];
    memset(e->nspace, 0, sizeof(e->nspace));
    strncpy(e->nspace, proc->nspace, PMIX_MAX_NSLEN);
    e->rank = proc->rank;
    memset(e->key, 0, sizeof(e->key));
    strncpy(e->key, key, PMIX_MAX_KEYLEN);
    stub_val_copy(&e->val, val);
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Init(pmix_proc_t *proc, const pmix_info_t info[], size_t ninfo)
{
    (void) info;
    (void) ninfo;
    if (NULL != proc) {
        *proc = stub_self;
        stub_have_self = 1;
    }
    /* tell Open MPI to take its singleton path */
    return PMIX_ERR_UNREACH;
}

pmix_status_t PMIx_Finalize(const pmix_info_t info[], size_t ninfo)
{
    (void) info;
    (void) ninfo;
    stub_have_self = 0;
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Put(pmix_scope_t scope, const char key[],
                       const pmix_value_t *val)
{
    (void) scope;
    if (NULL == key || NULL == val) {
        return PMIX_ERR_BAD_PARAM;
    }
    return stub_put(&stub_self, key, val);
}

pmix_status_t PMIx_Store_internal(const pmix_proc_t *proc, const char key[],
                                  pmix_value_t *val)
{
    if (NULL == proc || NULL == key || NULL == val) {
        return PMIX_ERR_BAD_PARAM;
    }
    return stub_put(proc, key, val);
}

pmix_status_t PMIx_Commit(void)
{
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Fence(const pmix_proc_t procs[], size_t nprocs,
                         const pmix_info_t info[], size_t ninfo)
{
    (void) procs;
    (void) nprocs;
    (void) info;
    (void) ninfo;
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Get(const pmix_proc_t *proc, const char key[],
                       const pmix_info_t info[], size_t ninfo,
                       pmix_value_t **val)
{
    int i;
    (void) info;
    (void) ninfo;
    if (NULL == val) {
        return PMIX_ERR_BAD_PARAM;
    }
    *val = NULL;
    if (NULL == proc || NULL == key) {
        return PMIX_ERR_BAD_PARAM;
    }
    for (i = 0; i < stub_nkv; i++) {
        stub_kv_t *e = &stub_kv[i];
        if (0 == strncmp(e->key, key, PMIX_MAX_KEYLEN)
            && 0 == strncmp(e->nspace, proc->nspace, PMIX_MAX_NSLEN)
            && (PMIX_RANK_WILDCARD == proc->rank || e->rank == proc->rank)) {
            pmix_value_t *v = malloc(sizeof(*v));
            if (NULL == v) {
                return PMIX_ERR_NOMEM;
            }
            stub_val_copy(v, &e->val);
            *val = v;
            return PMIX_SUCCESS;
        }
    }
    return PMIX_ERR_NOT_FOUND;
}

pmix_status_t PMIx_Get_nb(const pmix_proc_t *proc, const char key[],
                          const pmix_info_t info[], size_t ninfo,
                          pmix_value_cbfunc_t cbfunc, void *cbdata)
{
    pmix_value_t *v = NULL;
    pmix_status_t rc = PMIx_Get(proc, key, info, ninfo, &v);
    if (NULL != cbfunc) {
        cbfunc(rc, v, cbdata);
    }
    return PMIX_SUCCESS;
}

void PMIx_Value_free(pmix_value_t *v, size_t n)
{
    size_t i;
    if (NULL == v) {
        return;
    }
    for (i = 0; i < n; i++) {
        switch (v[i].type) {
        case PMIX_STRING:
            free(v[i].data.string);
            break;
        case PMIX_BYTE_OBJECT:
            free(v[i].data.bo.bytes);
            break;
        case PMIX_PROC:
            free(v[i].data.proc);
            break;
        case PMIX_ENVAR:
            free(v[i].data.envar.envar);
            free(v[i].data.envar.value);
            break;
        default:
            break;
        }
    }
    free(v);
}

pmix_status_t PMIx_Publish(const pmix_info_t info[], size_t ninfo)
{
    (void) info;
    (void) ninfo;
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Lookup(pmix_pdata_t data[], size_t ndata,
                          const pmix_info_t info[], size_t ninfo)
{
    (void) data;
    (void) ndata;
    (void) info;
    (void) ninfo;
    return PMIX_ERR_NOT_FOUND;
}

pmix_status_t PMIx_Register_event_handler(pmix_status_t codes[], size_t ncodes,
                                          pmix_info_t info[], size_t ninfo,
                                          pmix_notification_fn_t evhdlr,
                                          pmix_hdlr_reg_cbfunc_t cbfunc,
                                          void *cbdata)
{
    static size_t ref = 1;
    (void) codes;
    (void) ncodes;
    (void) info;
    (void) ninfo;
    (void) evhdlr;
    if (NULL != cbfunc) {
        cbfunc(PMIX_SUCCESS, ref++, cbdata);
    }
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Deregister_event_handler(size_t evhdlr_ref,
                                            pmix_op_cbfunc_t cbfunc, void *cbdata)
{
    (void) evhdlr_ref;
    if (NULL != cbfunc) {
        cbfunc(PMIX_SUCCESS, cbdata);
    }
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Notify_event(pmix_status_t status,
                                const pmix_proc_t *source,
                                pmix_data_range_t range,
                                const pmix_info_t info[], size_t ninfo,
                                pmix_op_cbfunc_t cbfunc, void *cbdata)
{
    (void) status;
    (void) source;
    (void) range;
    (void) info;
    (void) ninfo;
    if (NULL != cbfunc) {
        cbfunc(PMIX_SUCCESS, cbdata);
    }
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Log_nb(const pmix_info_t data[], size_t ndata,
                          const pmix_info_t directives[], size_t ndirs,
                          pmix_op_cbfunc_t cbfunc, void *cbdata)
{
    (void) data;
    (void) ndata;
    (void) directives;
    (void) ndirs;
    if (NULL != cbfunc) {
        cbfunc(PMIX_SUCCESS, cbdata);
    }
    return PMIX_SUCCESS;
}

pmix_status_t PMIx_Job_control_nb(const pmix_proc_t targets[], size_t ntargets,
                                  const pmix_info_t directives[], size_t ndirs,
                                  pmix_info_cbfunc_t cbfunc, void *cbdata)
{
    (void) targets;
    (void) ntargets;
    (void) directives;
    (void) ndirs;
    if (NULL != cbfunc) {
        cbfunc(PMIX_SUCCESS, NULL, 0, cbdata, NULL, NULL);
    }
    return PMIX_SUCCESS;
}

"""

# names the hand-written core provides -- skip auto-stubbing these
STUB_SKIP = set(re.findall(r"PMIx_\w+(?=\s*\()", STUB_CORE))


def ret_stmt(ret):
    r = ret.strip()
    r = re.sub(r"__pmix_attribute_\w+__\s*", "", r).strip()
    if r == "void" or r == "":
        return ""
    if "char" in r and "*" in r:
        return 'return "pmix-stub";'
    if "*" in r:
        return "return NULL;"
    if "status" in r or r in ("pmix_status_t", "pmix_status_code_t"):
        return "return PMIX_ERR_NOT_SUPPORTED;"
    return "return 0;"


def main():
    out = sys.argv[1]
    dirs = sys.argv[2:]
    decls = {}
    import glob
    import os

    headers = []
    for d in dirs:
        for path in sorted(glob.glob(os.path.join(d, "pmix*.h"))):
            try:
                text = open(path, encoding="utf-8", errors="replace").read()
            except OSError:
                continue
            collect_decls(text, decls)
            headers.append(os.path.basename(path))

    lines = [
        "/* Generated by cmake/gen_pmix_stub.py -- PMIx link stub.",
        " * Replaced by the real OpenPMIx build in Phase 4. */",
        "#define PMIX_BUILDING 1",
        "#include <pmix.h>",
    ]
    for h in headers:
        if h != "pmix.h":
            lines.append(f"#include <{h}>")
    lines += [
        "#include <stdlib.h>",
        "#include <string.h>",
        "",
        STUB_CORE,
        "",
        "/* autogen.pl output upstream; empty until real PMIx lands */",
        "__declspec(dllexport) char *pmix_framework_names[] = { NULL };",
        "",
    ]
    for name in sorted(decls):
        if name in STUB_SKIP:
            continue
        ret, args = decls[name]
        body = ret_stmt(ret)
        lines.append(f"{ret} {name}{args}")
        lines.append("{")
        if body:
            lines.append(f"    {body}")
        lines.append("}")
        lines.append("")

    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    print(f"gen_pmix_stub: wrote {len(decls)} stubs to {out}")


if __name__ == "__main__":
    main()
