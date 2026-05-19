// ============================================================
// FMSV Final Formal Specification — Auto-Generated
// Source: stage4_results.json [passed] only (BMC-proven lines)
// Initial passed: 25 | Vacuous: 0 | Subsumed: 3 | Final: 22
// Assert coverage (correctness):  7/7 assertable signals (100.0%)
// Cover  coverage (reachability): 7/7 assertable signals (100.0%)
// NOTE: cover() probes are reachability witnesses, not proofs of correctness.
// ============================================================

reg f_past_valid;
initial f_past_valid = 0;
always @(posedge clk) f_past_valid <= 1;

// --- Signal cluster: d_o ---
// [reachability] c_d_o_reach
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_d_o_reach: cover(d_o == 1'b1);

// [reachability] c_d_o_1
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_d_o_1: cover(d_o == 1'b1);

// [safety] a_d_o_tracks_d_in_1_r4
always @(posedge clk) begin
    if (f_past_valid)
        a_d_o_tracks_d_in_1_r4: assert(d_o == $past(d_in_1[0]));
end

// --- Signal cluster: d_o_reg ---
// [safety] a_d_o_tracks_d_o_reg
always @(posedge clk) begin
    if (f_past_valid)
        always @(posedge clk) if (f_past_valid) a_d_o_tracks_d_o_reg: assert(d_o == $past(d_o_reg));
end

// [reachability] c_d_o_reg_reach
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_d_o_reg_reach: cover(d_o_reg == 1'b1);

// [reachability] c_d_o_reg_1
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_d_o_reg_1: cover(d_o_reg == 1'b1);

// --- Signal cluster: nstate ---
// [liveness] a_nstate_stable
always @(posedge clk) begin
    if (f_past_valid)
        always @(posedge clk) if (f_past_valid) a_nstate_stable: assert($stable(nstate) || (nstate != $past(nstate)));
end

// [reachability] c_nstate_001
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_nstate_001: cover(nstate == 3'b001);

// [protocol] a_nstate_trans_001_to_010_r4
always @(posedge clk) begin
    if (f_past_valid)
        a_nstate_trans_001_to_010_r4: assert(nstate == 3'b010);
end

// [protocol] a_nstate_trans_001_to_010_r4_alt
always @(posedge clk) begin
    if (f_past_valid)
        a_nstate_trans_001_to_010_r4_alt: assert(nstate == 3'b010);
end

// --- Signal cluster: pstate ---
// [reachability] c_pstate_000
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_pstate_000: cover(pstate == 3'b000);

// [reachability] c_pstate_001
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_pstate_001: cover(pstate == 3'b001);

// [protocol] a_pstate_reset_r4
always @(posedge clk) begin
    if (f_past_valid)
        a_pstate_reset_r4: assert(pstate == 3'b000);
end

// --- Signal cluster: selection_buf ---
// [protocol] a_selection_buf_tracks
always @(posedge clk) begin
    if (f_past_valid)
        always @(posedge clk) if (f_past_valid) a_selection_buf_tracks: assert(selection_buf == $past(selection));
end

// [reachability] c_selection_buf_1
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_selection_buf_1: cover(selection_buf == 1'b1);

// --- Signal cluster: wr_en ---
// [reachability] c_wr_en_reach
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_wr_en_reach: cover(wr_en == 1'b1);

// [reachability] c_wr_en_1
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_wr_en_1: cover(wr_en == 1'b1);

// [safety] a_wr_en_func_r4
always @(posedge clk) begin
    if (f_past_valid)
        a_wr_en_func_r4: assert(wr_en == 1'b1);
end

// [protocol] a_wr_en_low_when_selection_low_r4
always @(posedge clk) begin
    if (f_past_valid)
        a_wr_en_low_when_selection_low_r4: assert(wr_en == 1'b0);
end

// --- Signal cluster: wr_en_reg ---
// [safety] a_wr_en_tracks_wr_en_reg
always @(posedge clk) begin
    if (f_past_valid)
        always @(posedge clk) if (f_past_valid) a_wr_en_tracks_wr_en_reg: assert(wr_en == $past(wr_en_reg));
end

// [reachability] c_wr_en_reg_reach
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_wr_en_reg_reach: cover(wr_en_reg == 1'b1);

// [reachability] c_wr_en_reg_1
always @(posedge clk)
    always @(posedge clk) if (f_past_valid) c_wr_en_reg_1: cover(wr_en_reg == 1'b1);

