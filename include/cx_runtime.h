typedef int128_t cx_id_t;        // global: CX ID, a 128b GUID
typedef int32_t cxu_id_t;        // system: CXU index
typedef int32_t state_id_t;      // system: state index
typedef int32_t cx_sel_t;        // hart: CX selector (value or index)

// CX discovery
static cxu_id_t get_cxu(cx_id_t); // map CX to its CXU here, if any

// state context management
static state_id_t alloc_state(cxu_id_t);
static void free_state(cxu_id_t, state_id_t);

// CX + state selectors
static cx_sel_t alloc_sel_cxu(cxu_id_t);
static cx_sel_t alloc_sel_cxu_state(cx_id_t);
static void free_sel(cx_sel_t);

// CX multiplexing
static cx_sel_t set_cur_sel(cx_sel_t); // return prev selector 

