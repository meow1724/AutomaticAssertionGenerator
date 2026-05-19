#!/usr/bin/env python3
"""
FMSV Pipeline v4 — Stage 1 + Stage 2
=====================================
Stage 1: Spec Processing (unchanged, proven working)
Stage 2: Batched Per-Signal Leader-Extender Assertion Generation (new)

Usage:
    python3 pipeline_s1s2.py --design GA_72 --genben ../GenBen
"""

import json, os, sys, time, re, argparse, math, requests


# ============================================
# GROQ CLIENT (with improved rate handling)
# ============================================
class GroqClient:
    BASE_URL = "https://api.groq.com/openai/v1/chat/completions"

    def __init__(self, config_path):
        with open(config_path) as f:
            cfg = json.load(f)
        self.keys = cfg["groq_api_keys"]
        self.models = {
            "leader": cfg["primary_model"],
            # Updated to prefer the extender_model if present in your config
            "extender": cfg.get("extender_model", cfg.get("fallback_model", cfg["primary_model"]))
        }
        self.ki = 0
        self.calls = 0

    def chat(self, system, user, role="leader", temp=0.2, max_tok=4096): # FIX 3: Lowered default max_tokens
        self.calls += 1
        model = self.models.get(role, self.models["leader"])

        # Pace every 2 calls
        if self.calls > 1 and self.calls % 2 == 0:
            wait = 20
            print(f"    [API] Pausing {wait}s (call #{self.calls})...")
            time.sleep(wait)

        payload = {
            "model": model, 
            "temperature": temp, 
            "max_tokens": max_tok,
            "messages": [
                {"role": "system", "content": system},
                {"role": "user", "content": user}
            ]
        }

        for _ in range(len(self.keys)):
            h = {"Authorization": f"Bearer {self.keys[self.ki]}", "Content-Type": "application/json"}
            try:
                r = requests.post(self.BASE_URL, headers=h, json=payload, timeout=90)
                
                # FIX 3: Explicitly handle 413 Payload Too Large without retrying blindly
                if r.status_code == 413:
                    raise ValueError(f"Payload Too Large (413). Try reducing batch size or max_tokens for model {model}.")
                
                if r.status_code == 429:
                    self.ki = (self.ki + 1) % len(self.keys)
                    time.sleep(10)
                    continue
                    
                r.raise_for_status()
                return r.json()["choices"][0]["message"]["content"]
            except Exception as e:
                # Catching general errors and 413s
                if isinstance(e, ValueError) and "Payload Too Large" in str(e):
                    raise e
                print(f"    [API] Error: {e}")
                self.ki = (self.ki + 1) % len(self.keys)
                time.sleep(10)

        # Final retry after long wait
        for wait in [60, 120]:
            print(f"    [API] All keys exhausted, waiting {wait}s...")
            time.sleep(wait)
            h = {"Authorization": f"Bearer {self.keys[0]}", "Content-Type": "application/json"}
            try:
                r = requests.post(self.BASE_URL, headers=h, json=payload, timeout=90)
                if r.status_code == 413:
                    raise ValueError("Payload Too Large (413).")
                if r.status_code == 429:
                    continue
                r.raise_for_status()
                return r.json()["choices"][0]["message"]["content"]
            except:
                continue
        raise RuntimeError("All API keys failed after retries")


def clean_llm(resp):
    """Strip markdown fences from LLM response."""
    r = resp.strip()
    if r.startswith("```"):
        r = "\n".join(r.split("\n")[1:])
        if r.rstrip().endswith("```"):
            r = r.rstrip()[:-3]
    return r.strip()


# ============================================
# STAGE 1: SPEC PROCESSING (proven working)
# ============================================
def stage1(client, rtl_code, spec_text, work_dir):
    print("\n" + "=" * 60)
    print("STAGE 1: SPECIFICATION PROCESSING")
    print("=" * 60)

    # 1A: Signal Mapper
    print("\n  [1A] Signal Mapper...")
    sm_resp = clean_llm(client.chat(
        "You analyze Verilog RTL and extract signals. Respond with ONLY valid JSON, no markdown.",
        f"""Extract all signals from this Verilog. Return JSON with:
- module_name (string)
- ports: array of {{name, direction, width, type}}
- internal_signals: array of {{name, width, type}}
- clock_signal (string)
- reset_signal (string)
- reset_polarity: "active_high" or "active_low"

VERILOG:
{rtl_code}

Respond ONLY with JSON:""",
        role="leader"
    ))
    sig_map = json.loads(sm_resp)
    n_ports = len(sig_map.get('ports', []))
    n_internal = len(sig_map.get('internal_signals', []))
    print(f"    {n_ports} ports, {n_internal} internals")

    # 1B: Spec Analyzer
    print("  [1B] Spec Analyzer...")
    sa_resp = clean_llm(client.chat(
        "You analyze hardware specs. Respond with ONLY valid JSON, no markdown.",
        f"""Extract signal-wise functional descriptions from this spec. Return JSON with:
- module_description (string, 1 paragraph)
- signals: array of {{name, functional_description, reset_behavior, dependencies, assertion_hints}}

SPECIFICATION:
{spec_text}

Respond ONLY with JSON:""",
        role="leader"
    ))
    spec_a = json.loads(sa_resp)
    print(f"    {len(spec_a.get('signals', []))} signal descriptions")

    # 1C: Merge
    print("  [1C] Merging into Information Bank...")
    mg_resp = clean_llm(client.chat(
        "You merge RTL signal data with spec descriptions. Respond with ONLY valid JSON, no markdown.",
        f"""Merge into unified information bank. Use RTL-accurate signal names.

SIGNAL MAP (from RTL):
{json.dumps(sig_map, indent=2)}

SPEC ANALYSIS (from NL):
{json.dumps(spec_a, indent=2)}

Return JSON with:
- module_name, clock, reset, reset_polarity, module_description
- signals: array of {{rtl_name, direction, width, type, functional_description, reset_behavior, dependencies, assertion_hints}}

Match signals by name similarity. Include ALL signals from RTL even if spec doesn't mention them.
Respond ONLY with JSON:""",
        role="leader", max_tok=4096
    ))
    info_bank = json.loads(mg_resp)

    # Save
    ib_path = os.path.join(work_dir, "information_bank.json")
    with open(ib_path, "w") as f:
        json.dump(info_bank, f, indent=2)

    n_sig = len(info_bank.get('signals', []))
    print(f"    Saved: {ib_path} ({n_sig} signals)")
    print(f"    Module: {info_bank.get('module_name')}")
    print(f"    Clock: {info_bank.get('clock')}, Reset: {info_bank.get('reset')} ({info_bank.get('reset_polarity')})")

    return info_bank


# ============================================
# STAGE 2: BATCHED PER-SIGNAL GENERATION
# ============================================

LEADER_SYSTEM = """You generate formal assertions for open-source SymbiYosys.

CRITICAL RULES:
1. NO concurrent SVA: no |-> , |=> , ## , disable iff, property/endproperty, sequence/endsequence
2. ONLY immediate assertions inside always @(posedge clk) blocks
3. All assertions using $past MUST be guarded by: if (f_past_valid)
4. Give each assertion a UNIQUE descriptive label
5. Use ONLY: $past(), $stable(), $rose(), $fell()
6. Use == for comparison, NEVER === (4-state equality not supported)
7. NEVER put if() inside assert(). The if guard goes OUTSIDE: if (cond) label: assert(expr);
8. NEVER assert values of INPUT signals (clk, rst, enable, selection, d_in_0, d_in_1) — inputs are unconstrained
9. Only assert values of OUTPUT and INTERNAL signals (d_o, wr_en, pstate, nstate, selection_buf, wr_en_reg, d_o_reg)
10. NEVER use $isunknown or any SystemVerilog-only functions

CORRECT PATTERNS:
// Reset clears FSM state (pstate is internal reg, assertable)
always @(posedge clk) begin
    if (f_past_valid && !$past(rst))
        a_pstate_reset: assert(pstate == 3'b000);
end

// Output tracks its registered source
always @(posedge clk) begin
    if (f_past_valid)
        a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
end

// Combinational output depends on current input
always @(posedge clk)
    a_wr_en_reg_sel: assert(selection == 1'b0 ? wr_en_reg == 1'b0 : 1'b1);

// State transition with proper guards
always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable)
        && !($past(selection_buf) && !$past(selection))
        && $past(pstate) == 3'b000 && !$past(selection) && $past(d_in_0[0]))
        a_trans_000_to_001: assert(pstate == 3'b001);
end

// Cover statement for reachability
always @(posedge clk) c_pstate_001: cover(pstate == 3'b001);

WRONG — NEVER DO:
assert(clk == 1'b1);              // WRONG: asserting input value
assert(rst == 1'b0);              // WRONG: asserting input value
assert(d_in_0 <= 8'hFF);          // WRONG: trivial, always true
assert(enable == 1'b1);           // WRONG: asserting input value
assert($stable(pstate));          // WRONG: FSM state changes
assert(signal === value);         // WRONG: use == not ===
assert(if (cond) expr);           // WRONG: if goes outside assert
assert($isunknown(signal));       // WRONG: not supported

OUTPUT FORMAT: Raw assertion code only. No module. No bind. No endmodule. No markdown. No backticks."""


EXTENDER_SYSTEM = LEADER_SYSTEM  # Same syntax rules — differentiation is in user prompt only


def create_signal_batch_prompt(signals, rtl_code, info_bank, is_extender=False):
    """Create a prompt for a batch of signals — only assertable (output/internal) signals."""
    signal_descriptions = []
    for sig in signals:
        direction = sig.get('direction', 'unknown')
        desc = f"  - {sig['rtl_name']} [{sig.get('width','?')}-bit {sig.get('type','?')}] ({direction})"
        desc += f"\n    Function: {sig.get('functional_description', 'unknown')}"
        if sig.get('reset_behavior') and sig['reset_behavior'] != 'null':
            desc += f"\n    Reset: {sig['reset_behavior']}"
        if sig.get('dependencies'):
            desc += f"\n    Depends on: {', '.join(sig['dependencies'])}"
        signal_descriptions.append(desc)

    clock = info_bank.get('clock', 'clk')
    reset = info_bank.get('reset', 'rst')
    polarity = info_bank.get('reset_polarity', 'active_low')
    reset_meaning = 'rst==0' if polarity == 'active_low' else 'rst==1'

    if is_extender:
        instruction = f"""Generate 2-3 assertions per signal below. Focus on:
- State transitions (if was in state X with input Y, must go to state Z)
- Registered output tracking (output == $past(source_reg))
- Enable/selection behavior
- Each assertion MUST have a unique label and be inside always @(posedge clk)
- Guard $past with if (f_past_valid)"""
    else:
        instruction = f"""Generate exactly 3 assertions and 1 cover per signal below:
1. Reset behavior: what value does this signal take when reset was active? Use: if (f_past_valid && !$past({reset})) assert(signal == reset_value);
2. Functional correctness: does the signal do what the spec says?
3. Relationship with other signals: how does this signal relate to its dependencies?
4. Cover: a reachable condition for this signal

You MUST generate for EVERY signal. Do not skip any."""

    prompt = f"""{instruction}

DESIGN: {info_bank.get('module_name', 'top')}
CLOCK: {clock} (posedge)
RESET: {reset} ({polarity} — {reset_meaning} means reset active)

IMPORTANT: These signals are OUTPUTS or INTERNAL registers. You CAN assert their values.
Input signals (clk, rst, enable, selection, d_in_0, d_in_1) are UNconstrained — NEVER assert their values.

SIGNALS TO GENERATE ASSERTIONS FOR:
{chr(10).join(signal_descriptions)}

RTL CODE:
{rtl_code}

Output ONLY the assertion code (no module, no bind, no markdown):"""

    return prompt


def stage2(client, info_bank, rtl_code, work_dir):
    print("\n" + "=" * 60)
    print("STAGE 2: BATCHED PER-SIGNAL GENERATION")
    print("=" * 60)

    signals = info_bank.get("signals", [])

    # Separate inputs from assertable signals
    input_signals = [s for s in signals if s.get('direction') == 'input']
    assertable_signals = [s for s in signals if s.get('direction') != 'input']

    input_names = [s['rtl_name'] for s in input_signals]
    assertable_names = [s['rtl_name'] for s in assertable_signals]

    print(f"\n  Total signals: {len(signals)}")
    print(f"  Inputs (skip assertions): {input_names}")
    print(f"  Assertable (output/internal): {assertable_names}")

    batch_size = 2 # FIX 2: Reduced from 4 to 2 to prevent LLM truncation
    n_batches = math.ceil(len(assertable_signals) / batch_size)
    print(f"  {len(assertable_signals)} assertable signals → {n_batches} batches of {batch_size}")

    all_assertions = []

    # Preamble: f_past_valid + initial assume
    clock = info_bank.get('clock', 'clk')
    reset = info_bank.get('reset', 'rst')
    polarity = info_bank.get('reset_polarity', 'active_low')
    reset_val = f"!{reset}" if polarity == "active_low" else reset

    preamble = f"""// === Auto-generated formal assertions ===
// Inputs (not asserted): {', '.join(input_names)}
// Assertable signals: {', '.join(assertable_names)}

reg f_past_valid;
initial f_past_valid = 0;
always @(posedge {clock}) f_past_valid <= 1;
initial assume({reset_val});
"""
    all_assertions.append(preamble)

    for batch_idx in range(n_batches):
        batch_start = batch_idx * batch_size
        batch_end = min(batch_start + batch_size, len(assertable_signals))
        batch_signals = assertable_signals[batch_start:batch_end]
        batch_names = [s['rtl_name'] for s in batch_signals]

        print(f"\n  [Batch {batch_idx + 1}/{n_batches}] Signals: {batch_names}")

        # Leader LLM
        print(f"    Leader generating...")
        leader_prompt = create_signal_batch_prompt(batch_signals, rtl_code, info_bank, is_extender=False)
        leader_resp = clean_llm(client.chat(LEADER_SYSTEM, leader_prompt, role="leader", temp=0.2))
        leader_resp = clean_assertions(leader_resp)

        n_leader = len(re.findall(r'\bassert\s*\(', leader_resp))
        n_cover_l = len(re.findall(r'\bcover\s*\(', leader_resp))
        print(f"    Leader: {n_leader} assertions, {n_cover_l} covers")
        all_assertions.append(f"\n// --- Batch {batch_idx + 1} Leader ({', '.join(batch_names)}) ---\n")
        all_assertions.append(leader_resp)

        # Extender LLM
        print(f"    Extender generating...")
        extender_prompt = create_signal_batch_prompt(batch_signals, rtl_code, info_bank, is_extender=True)
        extender_resp = clean_llm(client.chat(EXTENDER_SYSTEM, extender_prompt, role="extender", temp=0.3))
        extender_resp = clean_assertions(extender_resp)

        n_ext = len(re.findall(r'\bassert\s*\(', extender_resp))
        n_cover_e = len(re.findall(r'\bcover\s*\(', extender_resp))
        print(f"    Extender: {n_ext} assertions, {n_cover_e} covers")
        all_assertions.append(f"\n// --- Batch {batch_idx + 1} Extender ({', '.join(batch_names)}) ---\n")
        all_assertions.append(extender_resp)

    # Merge all
    merged = "\n".join(all_assertions)

    # Post-processing: fix common LLM mistakes
    # Replace === with ==
    merged = merged.replace("===", "==")
    # Remove lines with $isunknown
    merged = "\n".join(l for l in merged.split("\n") if "$isunknown" not in l)

    # Deduplication
    seen_asserts = set()
    deduped_lines = []
    for line in merged.split("\n"):
        stripped = line.strip()
        if re.search(r'\bassert\s*\(', stripped) or re.search(r'\bcover\s*\(', stripped):
            normalized = re.sub(r'\s+', ' ', stripped)
            if normalized in seen_asserts:
                deduped_lines.append(f"// [DEDUP] {stripped}")
                deduped_lines.append("        ;") # FIX 1: Added null statement to preserve Verilog block syntax
                continue
            seen_asserts.add(normalized)
        deduped_lines.append(line)

    merged = "\n".join(deduped_lines)

    # Final counts
    na = len(re.findall(r'\bassert\s*\(', merged))
    nc = len(re.findall(r'\bcover\s*\(', merged))
    nd = merged.count("[DEDUP]")

    # Save
    raw_path = os.path.join(work_dir, "assertions_raw_llm.sv")
    with open(raw_path, "w") as f:
        f.write(merged)

    print(f"\n  {'=' * 40}")
    print(f"  STAGE 2 SUMMARY")
    print(f"  {'=' * 40}")
    print(f"  Total assertions: {na}")
    print(f"  Total covers:     {nc}")
    print(f"  Duplicates removed: {nd}")
    print(f"  Batches processed:  {n_batches}")
    print(f"  Saved: {raw_path}")

    return merged


def clean_assertions(code):
    """Remove any module/endmodule/bind, concurrent SVA, backticks, and forbidden syntax."""
    lines = code.split("\n")
    cleaned = []
    skip = False
    for line in lines:
        s = line.strip()
        # Remove backtick wrappers
        if s.startswith('`') and s.endswith('`'):
            s = s[1:-1].strip()
            line = s
        if s.startswith('`'):
            s = s[1:].strip()
            line = s
        if s.endswith('`'):
            s = s[:-1].strip()
            line = s
        # Skip module blocks
        if re.match(r'^module\s+', s):
            skip = True
            continue
        if skip and s.startswith("endmodule"):
            skip = False
            continue
        if skip:
            continue
        # Remove forbidden patterns
        if s.startswith("bind "):
            continue
        if '|->' in s or '|=>' in s or 'disable iff' in s:
            continue
        if s.startswith('endmodule'):
            continue
        # Remove assert property (concurrent SVA)
        if 'assert property' in s:
            continue
        # Remove $error
        if '$error' in s:
            continue
        # Remove $isunknown
        if '$isunknown' in s:
            continue
        cleaned.append(line)
    return "\n".join(cleaned)


# ============================================
# MAIN
# ============================================
def main():
    parser = argparse.ArgumentParser(description="FMSV Pipeline v4 — Stage 1 + Stage 2")
    parser.add_argument("--design", help="GenBen design ID (e.g. GA_72)")
    parser.add_argument("--design-dir", help="Full path to design directory")
    parser.add_argument("--genben", default="../GenBen", help="GenBen repo root")
    parser.add_argument("--config", default="config.json", help="Config file path")
    args = parser.parse_args()

    if args.design:
        design_dir = os.path.join(args.genben, "data", "Design", args.design)
    elif args.design_dir:
        design_dir = args.design_dir
    else:
        print("ERROR: Provide --design or --design-dir")
        sys.exit(1)

    design_dir = os.path.abspath(design_dir)
    rtl_path = os.path.join(design_dir, "dut.v")
    spec_path = os.path.join(design_dir, "spec.txt")

    for p in [rtl_path, spec_path]:
        if not os.path.exists(p):
            print(f"ERROR: {p} not found")
            sys.exit(1)

    with open(rtl_path) as f:
        rtl_code = f.read()
    with open(spec_path) as f:
        spec_text = f.read()

    design_id = os.path.basename(design_dir)
    work_dir = os.path.abspath(f"run_{design_id}")
    os.makedirs(work_dir, exist_ok=True)

    print("╔" + "═" * 58 + "╗")
    print("║  FMSV PIPELINE v4 — Stage 1 + Stage 2                  ║")
    print("╚" + "═" * 58 + "╝")
    print(f"Design:  {design_id}")
    print(f"RTL:     {rtl_path} ({len(rtl_code)} chars)")
    print(f"Spec:    {spec_path} ({len(spec_text)} chars)")
    print(f"Work:    {work_dir}")

    config_path = os.path.abspath(args.config)
    if not os.path.exists(config_path):
        for p in ["../config.json", "../layer1/config.json"]:
            if os.path.exists(p):
                config_path = os.path.abspath(p)
                break
    client = GroqClient(config_path)

    # Stage 1
    info_bank = stage1(client, rtl_code, spec_text, work_dir)

    # Stage 2
    assertion_code = stage2(client, info_bank, rtl_code, work_dir)

    # Summary
    na = len(re.findall(r'\bassert\s*\(', assertion_code))
    nc = len(re.findall(r'\bcover\s*\(', assertion_code))

    print("\n" + "╔" + "═" * 58 + "╗")
    print("║  STAGES 1-2 COMPLETE                                   ║")
    print("╚" + "═" * 58 + "╝")
    print(f"Design:      {design_id}")
    print(f"Signals:     {len(info_bank.get('signals', []))}")
    print(f"Assertions:  {na}")
    print(f"Covers:      {nc}")
    print(f"API calls:   {client.calls}")
    print(f"\nOutputs in {work_dir}/:")
    print(f"  information_bank.json   — Stage 1")
    print(f"  assertions_raw_llm.sv   — Stage 2")
    print(f"\nNext: Run Stage 3-6 on these assertions")


if __name__ == "__main__":
    main()