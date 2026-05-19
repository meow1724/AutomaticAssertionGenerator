# FMSV Stage 6 — Final Formal Verification Report

## Summary

| Metric | Count |
|--------|-------|
| Initial properties | 25 |
| Vacuous (removed) | 0 |
| Subsumed (redundant) | 3 |
| **Final surviving** | **22** |
| Subsumption pairs tested | 12 |

## Property Breakdown

| Type | Count |
|------|-------|
| Assertions | 10 |
| Covers | 12 |

## Classification

| Class | Count |
|-------|-------|
| Safety | 4 |
| Liveness | 1 |
| Reachability | 12 |
| Protocol | 5 |

## Formal signal coverage

Assertable (non-input) signals from the information bank: **7**

Coverage is reported **separately** for asserts (correctness) and covers (reachability). A `cover` passing means the state is reachable — it is **not** a proof that the design behaves correctly. Only `assert` coverage reflects verified functional properties.

| Metric | Signals | Percent | Meaning |
|--------|---------|---------|---------|
| **Assert coverage (correctness)** | **7 / 7** | **100.0%** | Signals with a BMC-proven assertion |
| Cover coverage (reachability only) | 7 / 7 | 100.0% | Signals with a reachable `cover()` witness |
| Union (any surviving property) | 7 / 7 | 100.0% | Signals appearing in *any* surviving property |

### Signals with surviving asserts (correctness-verified)

- `d_o`
- `d_o_reg`
- `nstate`
- `pstate`
- `selection_buf`
- `wr_en`
- `wr_en_reg`

### Signals with only cover witnesses (reachability only)

None.

### Assertable signals not referenced in any surviving property

None.

## Signal Clusters

### `d_o` (3 properties)

- **c_d_o_reach** [reachability] `always @(posedge clk) if (f_past_valid) c_d_o_reach: cover(d_o == 1'b1);`
- **c_d_o_1** [reachability] `always @(posedge clk) if (f_past_valid) c_d_o_1: cover(d_o == 1'b1);`
- **a_d_o_tracks_d_in_1_r4** [safety] `a_d_o_tracks_d_in_1_r4: assert(d_o == $past(d_in_1[0]));`

### `d_o_reg` (3 properties)

- **a_d_o_tracks_d_o_reg** [safety] `always @(posedge clk) if (f_past_valid) a_d_o_tracks_d_o_reg: assert(d_o == $past(d_o_reg));`
- **c_d_o_reg_reach** [reachability] `always @(posedge clk) if (f_past_valid) c_d_o_reg_reach: cover(d_o_reg == 1'b1);`
- **c_d_o_reg_1** [reachability] `always @(posedge clk) if (f_past_valid) c_d_o_reg_1: cover(d_o_reg == 1'b1);`

### `nstate` (4 properties)

- **a_nstate_stable** [liveness] `always @(posedge clk) if (f_past_valid) a_nstate_stable: assert($stable(nstate) || (nstate != $past(nstate)));`
- **c_nstate_001** [reachability] `always @(posedge clk) if (f_past_valid) c_nstate_001: cover(nstate == 3'b001);`
- **a_nstate_trans_001_to_010_r4** [protocol] `a_nstate_trans_001_to_010_r4: assert(nstate == 3'b010);`
- **a_nstate_trans_001_to_010_r4_alt** [protocol] `a_nstate_trans_001_to_010_r4_alt: assert(nstate == 3'b010);`

### `pstate` (3 properties)

- **c_pstate_000** [reachability] `always @(posedge clk) if (f_past_valid) c_pstate_000: cover(pstate == 3'b000);`
- **c_pstate_001** [reachability] `always @(posedge clk) if (f_past_valid) c_pstate_001: cover(pstate == 3'b001);`
- **a_pstate_reset_r4** [protocol] `a_pstate_reset_r4: assert(pstate == 3'b000);`

### `selection_buf` (2 properties)

- **a_selection_buf_tracks** [protocol] `always @(posedge clk) if (f_past_valid) a_selection_buf_tracks: assert(selection_buf == $past(selection));`
- **c_selection_buf_1** [reachability] `always @(posedge clk) if (f_past_valid) c_selection_buf_1: cover(selection_buf == 1'b1);`

### `wr_en` (4 properties)

- **c_wr_en_reach** [reachability] `always @(posedge clk) if (f_past_valid) c_wr_en_reach: cover(wr_en == 1'b1);`
- **c_wr_en_1** [reachability] `always @(posedge clk) if (f_past_valid) c_wr_en_1: cover(wr_en == 1'b1);`
- **a_wr_en_func_r4** [safety] `a_wr_en_func_r4: assert(wr_en == 1'b1);`
- **a_wr_en_low_when_selection_low_r4** [protocol] `a_wr_en_low_when_selection_low_r4: assert(wr_en == 1'b0);`

### `wr_en_reg` (3 properties)

- **a_wr_en_tracks_wr_en_reg** [safety] `always @(posedge clk) if (f_past_valid) a_wr_en_tracks_wr_en_reg: assert(wr_en == $past(wr_en_reg));`
- **c_wr_en_reg_reach** [reachability] `always @(posedge clk) if (f_past_valid) c_wr_en_reg_reach: cover(wr_en_reg == 1'b1);`
- **c_wr_en_reg_1** [reachability] `always @(posedge clk) if (f_past_valid) c_wr_en_reg_1: cover(wr_en_reg == 1'b1);`

## Vacuous Properties (Removed)

None.

## Subsumed Properties (Removed)

- ~~a_pstate_stable~~ — `always @(posedge clk) if (f_past_valid) a_pstate_stable: assert($stable(pstate) || (pstate != $past(pstate)));`
- ~~a_wr_en_tracks~~ — `always @(posedge clk) if (f_past_valid) a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));`
- ~~a_d_o_tracks~~ — `always @(posedge clk) if (f_past_valid) a_d_o_tracks: assert(d_o == $past(d_o_reg));`

## Property retention (optimization)

**22 / 25** BMC-passed properties survived vacuity/subsumption = **88.0%** retention

## Pass Provenance & BMC Depth

Each property below was bounded-model-checked by SymbiYosys with `mode bmc, depth 20`. A `proven_depth` of N means the property held for every cycle `0..N-1` of the BMC trace — a proof at depth ≥ 2 is evidence the property is **not a Step-0 / reset-vector tautology**.

### Totals by BMC proof depth (all 25 initially-passed)

| Proven depth | Count | Interpretation |
|---|---|---|
| ≥ 2 cycles | 25 | Non-trivial: survived beyond Step 0 / reset-hold |
| = 1 cycle  | 0 | Only held at Step 0 — likely vacuous/reset-only |
| unknown / 0 | 0 | Depth not captured (legacy entries) |

Of the **22** survivors after vacuity + subsumption, **22** were proven at depth ≥ 2 cycles.

### Properties first proven per stage/round

This shows where in the pipeline each surviving property was *first* proven. `stage_4_initial` = proven on the very first Stage-2 LLM batch (before any refinement).

| Origin | First-passed | Survived Stage 6 | Vacuous | Subsumed |
|---|---|---|---|---|
| `stage_4_initial` | 19 | 16 | 0 | 3 |
| `stage_5_round_4` | 6 | 6 | 0 | 0 |
| **Cumulative total** | **25** | **22** | — | — |

> **Cumulative total passed = 25** (sum of all BMC-passed properties across Stage 4 initial + every Stage 5 refinement round + focused salvage).
> **Final surviving after Stage 6 (vacuity + subsumption + clustering) = 22**.

### Per-property provenance

| Label | Type | Origin | Proven depth | Status |
|---|---|---|---|---|
| `c_d_o_reach` | cover | `stage_4_initial` | 20 cycles | survived |
| `c_wr_en_reach` | cover | `stage_4_initial` | 20 cycles | survived |
| `a_d_o_tracks_d_o_reg` | assert | `stage_4_initial` | 20 cycles | survived |
| `a_wr_en_tracks_wr_en_reg` | assert | `stage_4_initial` | 20 cycles | survived |
| `c_d_o_reg_reach` | cover | `stage_4_initial` | 20 cycles | survived |
| `c_wr_en_reg_reach` | cover | `stage_4_initial` | 20 cycles | survived |
| `a_pstate_stable` | assert | `stage_4_initial` | 20 cycles | subsumed |
| `c_pstate_000` | cover | `stage_4_initial` | 20 cycles | survived |
| `a_nstate_stable` | assert | `stage_4_initial` | 20 cycles | survived |
| `c_nstate_001` | cover | `stage_4_initial` | 20 cycles | survived |
| `a_selection_buf_tracks` | assert | `stage_4_initial` | 20 cycles | survived |
| `c_selection_buf_1` | cover | `stage_4_initial` | 20 cycles | survived |
| `c_pstate_001` | cover | `stage_4_initial` | 20 cycles | survived |
| `a_wr_en_tracks` | assert | `stage_4_initial` | 20 cycles | subsumed |
| `c_wr_en_1` | cover | `stage_4_initial` | 20 cycles | survived |
| `c_wr_en_reg_1` | cover | `stage_4_initial` | 20 cycles | survived |
| `a_d_o_tracks` | assert | `stage_4_initial` | 20 cycles | subsumed |
| `c_d_o_1` | cover | `stage_4_initial` | 20 cycles | survived |
| `c_d_o_reg_1` | cover | `stage_4_initial` | 20 cycles | survived |
| `a_wr_en_func_r4` | assert | `stage_5_round_4` | 20 cycles | survived |
| `a_wr_en_low_when_selection_low_r4` | assert | `stage_5_round_4` | 20 cycles | survived |
| `a_pstate_reset_r4` | assert | `stage_5_round_4` | 20 cycles | survived |
| `a_d_o_tracks_d_in_1_r4` | assert | `stage_5_round_4` | 20 cycles | survived |
| `a_nstate_trans_001_to_010_r4` | assert | `stage_5_round_4` | 20 cycles | survived |
| `a_nstate_trans_001_to_010_r4_alt` | assert | `stage_5_round_4` | 20 cycles | survived |

---
*Generated by FMSV Pipeline v4 — Stage 6*
