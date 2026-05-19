// ---- Stage 5 formal preamble (regenerated every round) ----
reg f_past_valid;
initial f_past_valid = 0;
always @(posedge clk) f_past_valid <= 1;
reg [1:0] fmsv_init_cnt;
initial fmsv_init_cnt = 0;
always @(posedge clk) if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;
always @(*) if (fmsv_init_cnt < 2'd2) assume(!rst);

always @(posedge clk) if (f_past_valid) c_d_o_reach: cover(d_o == 1'b1);
always @(posedge clk) if (f_past_valid) c_wr_en_reach: cover(wr_en == 1'b1);
always @(posedge clk) if (f_past_valid) a_d_o_tracks_d_o_reg: assert(d_o == $past(d_o_reg));
always @(posedge clk) if (f_past_valid) a_wr_en_tracks_wr_en_reg: assert(wr_en == $past(wr_en_reg));
always @(posedge clk) if (f_past_valid) c_d_o_reg_reach: cover(d_o_reg == 1'b1);
always @(posedge clk) if (f_past_valid) c_wr_en_reg_reach: cover(wr_en_reg == 1'b1);
always @(posedge clk) if (f_past_valid) a_pstate_stable: assert($stable(pstate) || (pstate != $past(pstate)));
always @(posedge clk) if (f_past_valid) c_pstate_000: cover(pstate == 3'b000);
always @(posedge clk) if (f_past_valid) a_nstate_stable: assert($stable(nstate) || (nstate != $past(nstate)));
always @(posedge clk) if (f_past_valid) c_nstate_001: cover(nstate == 3'b001);
always @(posedge clk) if (f_past_valid) a_selection_buf_tracks: assert(selection_buf == $past(selection));
always @(posedge clk) if (f_past_valid) c_selection_buf_1: cover(selection_buf == 1'b1);
always @(posedge clk) if (f_past_valid) c_pstate_001: cover(pstate == 3'b001);
always @(posedge clk) if (f_past_valid) a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
always @(posedge clk) if (f_past_valid) c_wr_en_1: cover(wr_en == 1'b1);
always @(posedge clk) if (f_past_valid) c_wr_en_reg_1: cover(wr_en_reg == 1'b1);
always @(posedge clk) if (f_past_valid) a_d_o_tracks: assert(d_o == $past(d_o_reg));
always @(posedge clk) if (f_past_valid) c_d_o_1: cover(d_o == 1'b1);
always @(posedge clk) if (f_past_valid) c_d_o_reg_1: cover(d_o_reg == 1'b1);
always @(posedge clk) if (f_past_valid) a_d_o_reset_r3: assert(d_o == $past(d_in_1[0]));
always @(posedge clk) if (f_past_valid) a_d_o_rel_r3: assert(d_o == $past(d_in_1[0]));
always @(posedge clk) if (f_past_valid) a_wr_en_reset_r3: assert(wr_en == 1'b1);
always @(posedge clk) if (f_past_valid) a_trans_001_to_010_r3: assert(pstate == 3'b010);
always @(posedge clk) if (f_past_valid) a_trans_010_to_011_r3: assert(pstate == 3'b011);
always @(posedge clk) if (f_past_valid) a_pstate_trans_001_to_010_r3: assert(pstate == 3'b010);

// ---- Stage 5 round 4 refinements ----
always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_wr_en_func_r4: assert(wr_en == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_wr_en_low_when_selection_low_r4: assert(wr_en == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && !$past(enable))
        a_pstate_reset_r4: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_tracks_d_in_1_r4: assert(d_o == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_d_o_tracks_d_in_0_r4: assert(d_o == $past(d_in_0[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b000 && $past(selection) == 1'b1)
        a_nstate_trans_000_to_001_r4: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b1)
        a_nstate_trans_001_to_010_r4: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b000 && $past(selection) == 1'b0)
        a_nstate_trans_000_to_001_r4_alt: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b0)
        a_nstate_trans_001_to_010_r4_alt: assert(nstate == 3'b010);
end
