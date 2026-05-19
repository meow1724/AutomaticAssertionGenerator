# FMSV Pipeline v4 (C++)

**F**ormal **M**odel-checked **S**ystem **V**erification pipeline — a single C++ tool that takes an RTL design (`dut.v`) plus a natural-language specification (`spec.txt`) and produces a set of **bounded-model-check-proven SystemVerilog assertions** plus a human-readable verification report.

The pipeline is a cooperation between three things:

1. **LLMs (Groq)** — propose candidate assertions from the spec + RTL.
2. **Yosys** — syntax-check the candidate assertions against the RTL.
3. **SymbiYosys (`sby`) + Z3** — bounded-model-check each surviving candidate, produce counterexamples for the bad ones, re-prompt the LLM with those counterexamples, and iterate.

Everything after Stage 2 is rigorous: a property that appears in the final report has been *formally proven* to hold for every cycle `0..depth-1` of a BMC trace against the real RTL, not just "generated and sounded right."

---

## Table of contents

1. [Repository layout](#repository-layout)
2. [Prerequisites](#prerequisites)
3. [Configuration (`config.json`)](#configuration-configjson)
4. [Build](#build)
5. [Run](#run)
6. [Inputs and outputs at a glance](#inputs-and-outputs-at-a-glance)
7. [Pipeline stages — what each one does](#pipeline-stages--what-each-one-does)
   - [Stage 1 — Specification Processing](#stage-1--specification-processing)
   - [Stage 2 — Batched Per-Signal Generation](#stage-2--batched-per-signal-generation)
   - [Stage 3 — Yosys Compilation & Auto-Fix (+ Bisect Salvage)](#stage-3--yosys-compilation--auto-fix--bisect-salvage)
   - [Stage 4 — Exhaustive BMC (Isolation)](#stage-4--exhaustive-bmc-isolation)
   - [Stage 5 — LLM Refinement Loop (+ Focused Salvage)](#stage-5--llm-refinement-loop--focused-salvage)
   - [Stage 6 — Vacuity · Subsumption · Clustering · Report](#stage-6--vacuity--subsumption--clustering--report)
8. [Resume / partial runs (`--stage`)](#resume--partial-runs---stage)
9. [API key handling (Groq)](#api-key-handling-groq)
10. [Glossary](#glossary)
11. [Troubleshooting](#troubleshooting)

---

## Repository layout

```
pipeline_v4/
├── CMakeLists.txt              # CMake build config
├── pipeline_s1s2.cpp           # The entire pipeline (one translation unit)
├── config.json                 # Groq API keys + model names
├── build/                      # CMake build directory (you create this)
│   └── pipeline_s1s2           # Compiled binary
└── run_<design_id>/            # Per-design work directory (created at runtime)
    ├── information_bank.json   # Stage 1 output
    ├── assertions_raw_llm.sv   # Stage 2 output
    ├── assertions_compiled_clean.sv  # Stage 3 output
    ├── dut_with_asserts.sv     # RTL + injected assertions (for Yosys)
    ├── dut_temp.sv             # Per-property isolation file (Stage 4)
    ├── temp.sby                # SymbiYosys config (Stage 4)
    ├── stage4_results.json     # Per-property pass/fail ledger
    ├── stage4_history.json     # Origin tracking (which stage first proved each label)
    ├── failed_traces/          # Counterexamples (VCD + trace_tb.v) per failed label
    ├── stage5_*.sv / *.txt     # Stage 5 LLM prompts + raw outputs per round
    ├── stage5_refinement.json  # Stage 5 summary
    ├── stage5_vault_passes.sv  # Round-local vault of proven asserts
    ├── stage6_vacuity/         # Per-property reachability probes
    ├── stage6_subsumption/     # Pairwise A⊃B probes
    ├── stage6_results.json     # Final metrics
    ├── final_fmsv_spec.sv      # **The deliverable** — clean, clustered, proven assertions
    └── stage6_report.md        # **The deliverable** — human-readable report
```

---

## Prerequisites

The pipeline shells out to real open-source tools. You need:

| Tool | Used in | Install (macOS) |
|------|---------|-----------------|
| **CMake ≥ 3.14** | build | `brew install cmake` |
| **C++17 compiler** (Apple Clang works) | build | Xcode CLT |
| **nlohmann/json** | build | `brew install nlohmann-json` |
| **cpr** (C++ Requests) | build | `brew install cpr` |
| **Yosys** (open source) | Stage 3, 5 syntax check | `brew install yosys` |
| **SymbiYosys** (`sby`) | Stage 4, 5, 6 BMC | `brew install symbiyosys` |
| **Z3 SMT solver** | SymbiYosys engine | `brew install z3` |
| **Internet + Groq account** | Stages 1, 2, 3-autofix, 5 | get keys at [console.groq.com](https://console.groq.com) |

The pipeline expects `yosys` and `sby` to be in `PATH` (it explicitly prepends `/opt/homebrew/bin` and `/usr/local/bin` when invoking SymbiYosys).

---

## Configuration (`config.json`)

Put this at the repo root (`pipeline_v4/config.json`):

```json
{
  "groq_api_keys": [
    "gsk_...",
    "gsk_..."
  ],
  "primary_model":  "llama-3.3-70b-versatile",
  "extender_model": "llama-3.3-70b-versatile",
  "fallback_model": "llama-3.1-8b-instant"
}
```

- `groq_api_keys` — one or more Groq API keys. The client rotates through them on HTTP 429 (rate limit). More keys ≠ more throughput if they belong to the same Groq org (org-level TPM caps apply).
- `primary_model` — used by Stage 1, Stage 2 "leader", Stage 3 auto-fix, Stage 5.
- `extender_model` — used by Stage 2 "extender" role (generates extra variations).
- `fallback_model` — currently only used as the extender fallback when `extender_model` is absent.

---

## Build

```bash
cd pipeline_v4
mkdir -p build
cd build
cmake ..
make -j
```

Produces `build/pipeline_s1s2`.

---

## Run

From `build/`:

```bash
./pipeline_s1s2 --design GA_75 --genben ../../GenBen
```

All CLI flags:

| Flag | Meaning | Default |
|------|---------|---------|
| `--design <id>` | Design name; resolves to `<genben>/data/Design/<id>/` | *(required unless `--design-dir`)* |
| `--design-dir <path>` | Full path to a design directory containing `dut.v` and `spec.txt` | — |
| `--genben <path>` | Root of the GenBen benchmark | `../GenBen` |
| `--config <path>` | Path to `config.json` | `config.json`, falls back to `../config.json` |
| `--stage <N>` | Start from stage `N` (resume mode, `N ∈ {1..6}`) | `1` |

Output goes to `../run_<design_id>/` (relative to the binary's CWD).

---

## Inputs and outputs at a glance

**Inputs (per design):**

- `dut.v` — the RTL to verify.
- `spec.txt` — natural-language spec (Markdown / plain text).

**Outputs (per design, in `run_<design_id>/`):**

- `final_fmsv_spec.sv` — the canonical deliverable: all BMC-proven, non-vacuous, non-subsumed assertions, clustered by driving signal, with classification comments (`safety` / `liveness` / `reachability` / `protocol`).
- `stage6_report.md` — Markdown report with pass/fail tables, provenance, classification breakdown, and signal coverage.
- `stage6_results.json` — machine-readable equivalent of the report.

All other files are intermediate artifacts useful for debugging.

---

## Pipeline stages — what each one does

The whole flow runs in one process by default. Every stage writes its outputs to `run_<design_id>/` so later stages can resume from disk.

### Stage 1 — Specification Processing

> Function: `stage1()` · Output: `information_bank.json`

Three LLM calls in sequence:

- **[1A] Signal Mapper.** Sends the Verilog source and asks the LLM to extract a structural summary — ports, directions, widths, types, internal signals, clock, reset, and reset polarity. Pure structure, no behavioral reasoning.
- **[1B] Spec Analyzer.** Sends the natural-language spec and asks for per-signal *functional* descriptions: reset behavior, dependencies, assertion hints.
- **[1C] Merger.** Sends both previous JSONs back to the LLM and asks for a unified **information bank** keyed by RTL-accurate signal names. Signals appearing in the RTL but not the spec are still included (unknown function is acceptable; missing signals are not).

The merged JSON is saved as `information_bank.json` — every downstream stage reads from this, not from the spec or RTL directly.

Why three calls instead of one? Keeping structural extraction separate from semantic analysis prevents the LLM from hallucinating signals that aren't in the RTL, which is the single biggest source of downstream failures.

### Stage 2 — Batched Per-Signal Generation

> Function: `stage2()` · Output: `assertions_raw_llm.sv`

This is where candidate assertions are actually written. Goals:

1. **Generate per-signal.** For each *assertable* signal (outputs + internal registers, never pure inputs), the LLM is prompted with that signal's info-bank entry plus the full RTL, and asked to emit assertions grounded in the actual RTL.
2. **Split into small batches.** Signals are batched 2 at a time. Smaller batches keep the prompt short and let each batch cost one LLM call rather than one per signal.
3. **Two LLM roles per batch** — "leader" and "extender":
   - *Leader* writes exactly 3 asserts + 1 cover per signal: reset behavior, functional correctness, relationship with dependencies, reachability cover.
   - *Extender* writes 2–3 additional asserts focused on state transitions, registered tracking, and enable/selection behavior.
   The two roles run back-to-back per batch, so each signal typically gets 5–6 candidate properties.
4. **Prepended "formal preamble".** Before the first batch, the pipeline emits:
   - `reg f_past_valid;` initialized to 0, set to 1 after the first posedge — guards all `$past` references so BMC doesn't see garbage at step 0.
   - `reg [1:0] fmsv_init_cnt;` counter + an `assume(!reset)` block that **holds the reset line active for the first two cycles**. Without this, async resets wouldn't have time to clear FSM state before BMC starts checking assertions.
5. **Cleaning pass.** Every LLM response goes through `clean_llm()` (strip markdown / backticks / preamble) and `clean_assertions()` which removes Yosys-incompatible constructs: `|->`, `|=>`, `##`, `disable iff`, `property ... endproperty`, `sequence`, `assert property`, `bind`, unexpected `module/endmodule` headers. Those constructs belong to concurrent SVA which Yosys's open-source flow does not implement.
6. **Deduplication.** Exact and near-duplicate assertion lines are merged across batches.

The resulting artifact (`assertions_raw_llm.sv`) is a flat `.sv` file containing 50–200 candidate properties wrapped in procedural `always` blocks. It is not guaranteed to compile yet — that's Stage 3.

Prompt discipline (enforced in the system prompt):

- Immediate assertions only (inside `always @(posedge clk)`), no concurrent SVA.
- `$past` usage must be guarded by `if (f_past_valid)`.
- `==`, never `===`.
- `if` guard is always **outside** the `assert()` call, never inside.
- Never assert values of input signals.
- Unique label per assertion.
- No `$isunknown` or other SV-only builtins.
- Tautology ban: the guard cannot contain the same post-condition it is about to assert.

### Stage 3 — Yosys Compilation & Auto-Fix (+ Bisect Salvage)

> Function: `stage3()` · Output: `assertions_compiled_clean.sv`

The goal here is a single artifact that **compiles cleanly with Yosys** when injected into the RTL. Algorithm:

1. **Inject** the assertions just above `endmodule` in the RTL and write `dut_with_asserts.sv`.
2. **Compile** with `yosys -p "read_verilog -sv dut_with_asserts.sv; prep -top <module>"`.
3. **If it compiles** → save to `assertions_compiled_clean.sv` and return success.
4. **If it fails** → the tail of the Yosys error log (last 2 KB) is sent back to the LLM with the full RTL+assertion file, asking it to fix *only* the assertions. The fix is cleaned again.
5. **Guard against over-stripping.** If the LLM's "fix" reduces the assertion count by more than half, the fix is rejected and the previous content is retried (better to fail syntax than to pass with zero properties).
6. Up to **3 auto-fix attempts**.

If all 3 attempts fail, the pipeline no longer gives up — it enters **bisect salvage**:

- The assertion file is split into atomic blocks by a **property-anchored splitter** (`s3_split_blocks`). For each `assert(` / `cover(` token it finds the enclosing `always` / `initial` statement (handling both `begin/end` blocks and single-statement `always @(...) label: assert(...);` forms, including the common two-line variant) and emits it as one block. Text outside those ranges (reg declarations, initial blocks, the preamble, comments) becomes "infra" blocks which are always kept together.
- An **infra sanity probe** runs first — if infrastructure alone doesn't compile, the pipeline fails loudly instead of dropping every property one by one.
- Then a **recursive bisection** runs: combine all properties with infra → compile → if OK keep everything, if fail split the property list in half and recurse. Singletons that fail are dropped. Two halves that each compile individually but fail when combined (typically a duplicate label across halves) degrade gracefully: the larger half is kept.
- The surviving subset, plus a `// === [STAGE 3 SALVAGE] X of Y blocks dropped ===` marker, is saved as `assertions_compiled_clean.sv` and the pipeline continues.

Only if **zero** properties survive (or infra itself is broken) does Stage 3 give up and fail.

### Stage 4 — Exhaustive BMC (Isolation)

> Function: `stage4()` · Output: `stage4_results.json`, `failed_traces/*`

This is the first stage that actually uses a formal engine. The goal is to produce a rigorous per-property verdict: PASS, FAIL (with counterexample), or ERROR.

Algorithm:

1. Read `assertions_compiled_clean.sv` and identify every line containing `assert` or `cover` (excluding comments). Call these the `target_indices`.
2. For each target line:
   - **Isolate it.** Rewrite the file such that every *other* target line is replaced by `begin end // [ISOLATED] ...` (a no-op that still satisfies surrounding `if` blocks). Only the single target under test remains active.
   - **Inject** the isolated set into the RTL, write `dut_temp.sv`.
   - Write a `temp.sby` config:
     ```
     [options]
     mode bmc
     depth 20
     [engines]
     smtbmc z3
     [script]
     read_verilog -sv dut_temp.sv
     prep -top <module>
     [files]
     dut_temp.sv
     ```
   - Run `sby -f temp.sby`.
   - Parse the log:
     - `DONE (PASS)` → read every `Checking assertions in step N` line, take the max; **proven depth** = max+1. This is the *actual number of BMC cycles the property survived*, which is what distinguishes a real proof (depth ≥ 2) from a reset-cycle tautology (depth = 1).
     - `DONE (FAIL)` → extract the failing step and copy the trace into `failed_traces/<label>_trace.vcd` (for waveform viewers) and `failed_traces/<label>_trace_tb.v` (compact readable trace — used by Stage 5 to re-prompt the LLM).
     - Anything else → marked ERROR (engine crash, vacuity).
3. Write `stage4_results.json` with two buckets:
   ```json
   {
     "passed": [{"label", "original_code", "bmc_depth", "proven_depth", "origin", "first_proven_depth", "best_proven_depth"}, ...],
     "failed": [{"label", "failed_step", "trace_file", "trace_tb_file", "original_code"}, ...]
   }
   ```
4. Maintain `stage4_history.json` — a per-label ledger that records the *earliest* stage/round a label was first proven. This powers the `origin` column in Stage 6's final report, so you can see which properties came from the raw Stage 2 output vs. later refinement rounds.

Why isolate each assertion? A failure in one property can cause SymbiYosys to stop checking later properties in the same run. Isolation guarantees every property is independently verified and no failures are hidden by earlier ones.

### Stage 5 — LLM Refinement Loop (+ Focused Salvage)

> Function: `stage5()` · Output: updated `stage4_results.json`, `stage5_*.sv` artifacts

Runs only if Stage 4 reported failures. Otherwise it's skipped.

Two phases:

#### Phase A: broad refinement, up to 4 rounds

For each round:

1. **Rebuild the vault** from `stage4_results.json[passed]`. The "vault" is the set of properties that have *already been BMC-proven*. Two vaults are actually built:
   - *Prompt vault* — asserts only, no covers. Kept tight to fit in the LLM context.
   - *Injection vault* — full (asserts + covers) so cover coverage isn't lost across rounds.
2. **Build the prompt** containing:
   - Compact design hints (derived from `information_bank.json`).
   - The prompt vault (reference only — LLM is told not to repeat it).
   - The failed assertions' source lines.
   - For up to 8 failures: a short counterexample trace (extracted from `trace_tb.v` — per-cycle `PI_<input> = <value>` lines).
   - A list of assertable signals **still lacking a passing assert** ("uncovered signals") — this nudges the LLM to write new properties rather than endlessly refining the same ones.
   - The RTL (truncated to 14 KB if large).
3. **Temperature ramps** each round: `0.2 → 0.4 → 0.55 → 0.7`. Later rounds explicitly encourage divergence; otherwise the LLM re-emits the same broken assertions.
4. **Rebuild the formal preamble** every round (f_past_valid, fmsv_init_cnt, assume-reset block). Without this, resumption rounds leave `f_past_valid` as an undriven wire that BMC can set to 1 at step 0, spuriously firing every guarded assertion.
5. **Concatenate**: preamble + full injection vault + new LLM refinements → run `stage3()` → run `stage4()` with `origin_tag = "stage_5_round_N"`.
6. **Regression guards:**
   - If Stage 3 over-strips (clean output has 0 properties) → restore previous `stage4_results.json`, skip the round.
   - If this round didn't reduce failure count → keep trying (stall recovery via hotter temperature); only stop at `max_rounds`.
7. **High-water mark.** After each round, track the best-ever `passed` count and snapshot the full vault. Never regress below it.

#### Phase B: focused salvage

After broad refinement, compute `uncovered_signals(info_bank, current_vault)` — assertable signals that still have zero passing asserts. For each, emit **one targeted round** that:

1. Builds a prompt asking for 3–5 asserts *about that one signal*, including FSM transition hints if a `case(<signal>)` block is found in the RTL.
2. Runs the LLM at `temp=0.3`.
3. Filters the output to lines that textually mention the target signal (`filter_properties_mentioning`) — prevents the LLM from drifting to other signals.
4. Concatenates `preamble + current vault + filtered refinements` → Stage 3 → Stage 4 (origin tag `stage_5_salvage:<signal>`).
5. Accepts the result only if *that specific signal* now has a passing assert AND total passing count went up; otherwise restores the snapshot.

Focused salvage usually lifts assert-coverage by 5–15 percentage points on designs where broad rounds plateau.

### Stage 6 — Vacuity · Subsumption · Clustering · Report

> Function: `stage6()` · Output: `final_fmsv_spec.sv`, `stage6_report.md`, `stage6_results.json`

Stage 6 runs unconditionally after Stage 4 (same run), regardless of whether Stage 5 cleared every failure. It operates **only on the `passed[]` set** — failed properties never make it into the final spec.

Five passes:

#### [6A] Vacuity check

Two detectors:

1. **Tautology detector.** Parses `dut_with_asserts.sv` to recover each label's guard (the `if (...)` preceding it). Flags any assertion whose *inner asserted expression* appears as a conjunct of the guard. Example caught:
   ```verilog
   if (f_past_valid && d_o == 1'b0)  a_bad: assert(d_o == 1'b0);
   ```
   The property "passes" BMC but proves nothing — the guard already constrained the outcome. Flagged as vacuous.

2. **Reachability check.** For every non-vacuous assert, the pipeline writes an equivalent `cover(...)` (same guard, cover instead of assert) and runs SymbiYosys in `mode cover, depth 20`. If the cover is **unreachable**, the original guard is never satisfiable and the assert passes vacuously. Flagged.

#### [6B] Subsumption engine

Pairwise test: for each ordered pair `(A, B)` of non-vacuous asserts targeting the **same primary signal**, build a mini-design where `assume(A_expr)` is enforced and `assert(B_expr)` is checked. Run BMC at depth 10. If that passes, `A ⊃ B` — `B` is redundant and flagged subsumed.

One guard: if `A` is already flagged subsumed (by some earlier iteration), skip using it as the assume. This prevents equivalence-class wipe-out (`A ≡ B ≡ C` → all three marked subsumed).

Only the "primary signal" heuristic is used to prune the O(N²) search. Works well for small/medium designs (tens of asserts per signal).

#### [6C] Classification

Each surviving property is tagged as one of:

- **safety** — a classic invariant (`assert(x == ...)`, `assert($past(...) ==> ...)` equivalent).
- **liveness** — something is eventually true (rarely seen in immediate assertions; usually caught only in special cases).
- **reachability** — cover statements.
- **protocol** — pattern-matches multi-signal handshake checks.

#### [6D] Clustering

Group surviving properties by the primary signal they assert about. Output is organized by cluster in `final_fmsv_spec.sv`.

#### [6E] Emit `final_fmsv_spec.sv`

The deliverable. Structure:

```verilog
// FMSV Final Formal Specification — Auto-Generated
// Initial passed: N | Vacuous: V | Subsumed: S | Final: F
// Assert coverage: A/T ... Cover coverage: C/T ...
reg f_past_valid; ...

// --- Signal cluster: <signal> ---
// [safety] label_1
always @(posedge clk) begin
    if (f_past_valid)
        label_1: assert(...);
end
...
```

#### [6F] Coverage metrics

Three coverage figures are reported *separately* because they mean very different things:

- **Assert coverage (correctness)** — `# assertable signals with a passing assert` / `# assertable signals`. This is the only number that reflects formally-verified correctness.
- **Cover coverage (reachability only)** — signals with a passing `cover()`. Reachability is *not* correctness; a cover passing means "this state is reachable," nothing more.
- **Union coverage** — signals appearing in any surviving property (legacy field).

The report explicitly warns when surviving asserts are zero (only covers remained).

#### [6G] Pass-provenance breakdown

For each origin tag in `stage4_history.json` the report shows how many properties were first proven at that origin, how many survived Stage 6 pruning, how many were vacuous / subsumed. Depth breakdown (≥2 cycles vs. 1 cycle vs. unknown) is also reported so reviewers can distinguish non-trivial proofs from reset-vector tautologies.

---

## Resume / partial runs (`--stage`)

The pipeline supports starting at any stage `≥ 2`, provided the artifacts from earlier stages exist in `run_<design_id>/`.

| `--stage N` | What runs |
|---|---|
| `1` (default) | Full pipeline: 1 → 2 → 3 → 4 → 5 (if needed) → 6 |
| `2` | Same as 1 (Stage 1 cannot be skipped because the info-bank is the foundation) |
| `3` | Re-inject + Yosys + auto-fix + bisect; then Stage 4 → 5 → 6. Expects `assertions_raw_llm.sv` on disk. |
| `4` | Run BMC only, then Stage 5 (if needed) + Stage 6. Expects `assertions_compiled_clean.sv` + `information_bank.json`. |
| `5` | Skip 1–4. Read existing `stage4_results.json`. Run Stage 5 refinement (only if failures exist). Then Stage 6. |
| `6` | Skip everything, just rebuild the final spec + report from the current `stage4_results.json`. Useful after manual edits to passed/failed. |

In resume mode the binary prints `[RESUME] Stage N entry: ...` so you can confirm what it loaded.

---

## API key handling (Groq)

The `GroqClient` class manages:

- **Config discovery.** `config.json` in CWD, else `../config.json`, else whatever `--config` points to.
- **Key merging.** When using the default config name, keys in `./config.json` and `../config.json` are both ingested and deduplicated — so having a stub config in `build/` and a full one at the repo root both work.
- **Rotation policy.** On HTTP 429, rotate to the next key *immediately* (no sleep) until a full cycle has been tried. Only after all keys in the cycle have been hit do we sleep 30s and restart rotation. Retry budget scales with key count: `max(48, keys*12)` HTTP tries per call.
- **Call pacing.** Every other call sleeps 20s before firing to stay under Groq's TPM limits without relying on 429 handling.
- **Error surfacing.** For HTTP 400 the Groq error body is parsed and printed (message + code + type) — so "model decommissioned," "context overflow," "organization restricted" etc. are visible rather than lost to an empty `cpr error`. For 401/403 we rotate once and then fail fast. For 4xx/5xx with a body we log the body.

---

## Glossary

- **`f_past_valid`** — a register initialized to 0 and set to 1 after the first clock edge. Guarding `$past` with `if (f_past_valid)` prevents `$past` from evaluating against undefined pre-time-zero state.
- **`fmsv_init_cnt`** — a 2-bit counter + an `assume(reset_active)` for the first 2 cycles. Gives async resets time to propagate before BMC begins checking assertions.
- **BMC (bounded model checking)** — the engine (z3) exhaustively explores every possible input sequence for the first `depth` cycles and returns PASS only if no counterexample is found within that bound.
- **Proven depth** — the highest cycle number BMC checked before concluding PASS. A property passing at depth 1 held only at reset — usually uninteresting. A property passing at depth ≥ 2 is genuinely non-trivial.
- **Vacuous property** — a property that passes because its precondition is unsatisfiable or because the guard already implies the conclusion. "Passes" but proves nothing.
- **Subsumed property** — a property logically implied by another passing property on the same signal. Removed to shrink the final spec without losing coverage.
- **Vault** — the running set of BMC-proven asserts maintained across Stage 5 rounds. Every round's output = preamble + vault + LLM refinements, so proven properties aren't re-hallucinated.
- **Origin tag** — `stage_4_initial`, `stage_5_round_N`, `stage_5_salvage:<signal>`. Per-label provenance of where the property was first proven.

---

## Troubleshooting

| Symptom | Likely cause | Where to look |
|---|---|---|
| `[API] HTTP 400 ... organization_restricted` | Groq account/org flagged | log into console.groq.com; email support@groq.com |
| `[API] HTTP 400 ... model_decommissioned` | Old model name in config | Update `primary_model` / `extender_model` to a currently-supported Groq model |
| `[API] All N key(s) rate-limited, sleeping 30s` repeating forever | All keys in same org, TPM exhausted | Space calls more, lower batch count, or use keys from a different org |
| `[FATAL] Infrastructure block alone fails Yosys syntax check` | Auto-fix or Stage 5 corrupted the preamble | Inspect `assertions_compiled_clean.sv`; likely missing `reg f_past_valid;` or unbalanced `end` |
| `Stage 3 Compilation: FAIL` + bisect dropped many | Genuine syntax bugs in candidates (nested `$past`, unsupported functions) | Look at `dut_with_asserts.sv`, run `yosys -p "read_verilog -sv dut_with_asserts.sv"` manually to see errors |
| `Stage 4: total_asserts = 0` | Stage 3 over-stripped; or `clean_assertions()` removed everything | Re-run Stage 3 alone with `--stage 3`; inspect intermediate files |
| Stage 4 says `ERROR (Syntax/Vacuity)` for every property | `sby` / `z3` not in PATH | Verify `which sby` and `which yosys` succeed; pipeline prepends `/opt/homebrew/bin` and `/usr/local/bin` only |
| Stage 6 reports `0 / N` assert coverage | Every surviving property is a cover | Stage 5 didn't find refinable asserts; check `stage5_focus_*_raw.sv` for why salvage didn't help |
| Unexpected design directory / wrong files | `--genben` points to the wrong parent | Verify `--genben <path>` contains `data/Design/<id>/dut.v` and `spec.txt` |

For deeper debugging, every stage persists its prompts and LLM responses so you can replay:

- `stage5_last_user_prompt.txt` — exact prompt of the most recent Stage 5 LLM call.
- `stage5_llm_raw_r<N>.sv` — raw LLM output for round N (before `clean_assertions`).
- `stage5_focus_<signal>_prompt.txt`, `stage5_focus_<signal>_raw.sv`, `stage5_focus_<signal>_filtered.sv` — full trace of a focused-salvage attempt for one signal.
- `failed_traces/<label>_trace_tb.v` — the compact per-cycle input trace that refuted that label.

---

*FMSV Pipeline v4 — built around Groq LLMs, Yosys, and SymbiYosys/z3.*
