always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_func_r1: assert(d_o == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_d_o_rel_r1: assert(d_o == $past(d_in_0[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_wr_en_reset_r1: assert(wr_en == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1)
        a_trans_000_to_001_r1: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(d_in_1[0]) == 1'b1)
        a_trans_000_to_001_r2: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b001 && $past(d_in_0[1]) == 1'b1)
        a_trans_001_to_010_r1: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001 && $past(d_in_1[1]) == 1'b1)
        a_trans_001_to_010_r2: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b010 && $past(d_in_0[2]) == 1'b1)
        a_trans_010_to_011_r1: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b010 && $past(d_in_1[2]) == 1'b1)
        a_trans_010_to_011_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst))
        a_d_o_reg_reset_r1: assert(d_o_reg == $past(d_o_reg));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_wr_en_reg_func_r1: assert(wr_en_reg == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_wr_en_reg_func_r2: assert(wr_en_reg == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_reg_func_r1: assert(d_o_reg == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_d_o_reg_func_r2: assert(d_o_reg == $past(d_in_0[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_tracks_d_o_reg_r1: assert(d_o == $past(d_o_reg));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_d_o_tracks_d_o_reg_r2: assert(d_o == $past(d_o_reg));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_wr_en_tracks_wr_en_reg_r1: assert(wr_en == $past(wr_en_reg));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_wr_en_tracks_wr_en_reg_r2: assert(wr_en == $past(wr_en_reg));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_pstate_trans_000_to_001_r1: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b000 && $past(d_in_0[0]) == 1'b1)
        a_pstate_trans_000_to_001_r2: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001 && $past(d_in_1[1]) == 1'b1)
        a_pstate_trans_001_to_010_r1: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b001 && $past(d_in_0[1]) == 1'b1)
        a_pstate_trans_001_to_010_r2: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b010 && $past(d_in_1[2]) == 1'b1)
        a_pstate_trans_010_to_011_r1: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b010 && $past(d_in_0[2]) == 1'b1)
        a_pstate_trans_010_to_011_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_nstate_reset_r1: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_nstate_reset_r2: assert(nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001 && $past(d_in_1[1]) == 1'b1)
        a_nstate_trans_001_to_0_r1: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b001 && $past(d_in_0[1]) == 1'b1)
        a_nstate_trans_001_to_0_r2: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b010 && $past(d_in_1[2]) == 1'b1)
        a_trans_010_to_011_r1: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b010 && $past(d_in_0[2]) == 1'b1)
        a_trans_010_to_011_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_selection_buf_tracks_r1: assert(selection_buf == $past(selection));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_selection_buf_tracks_r2: assert(selection_buf == $past(selection));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_pstate_trans_001_to_010_r1: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b001 && $past(d_in_0[1]) == 1'b1)
        a_pstate_trans_001_to_010_r2: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b010 && $past(d_in_1[2]) == 1'b1)
        a_pstate_trans_010_to_011_r1: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b010 && $past(d_in_0[2]) == 1'b1)
        a_pstate_trans_010_to_011_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_pstate_reset_to_nstate_r1: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_pstate_reset_to_nstate_r2: assert(nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001 && $past(d_in_1[1]) == 1'b1)
        a_nstate_000_sel_0_r1: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b001 && $past(d_in_0[1]) == 1'b1)
        a_nstate_000_sel_0_r2: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b010 && $past(d_in_1[2]) == 1'b1)
        a_nstate_001_sel_0_r1: assert(nstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b010 && $past(d_in_0[2]) == 1'b1)
        a_nstate_001_sel_0_r2: assert(nstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_nstate_000_sel_1_r1: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_nstate_000_sel_1_r2: assert(nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001 && $past(d_in_1[1]) == 1'b1)
        a_nstate_001_sel_1_r1: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b001 && $past(d_in_0[1]) == 1'b1)
        a_nstate_001_sel_1_r2: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b010 && $past(d_in_1[2]) == 1'b1)
        a_nstate_010_sel_1_r1: assert(nstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b010 && $past(d_in_0[2]) == 1'b1)
        a_nstate_010_sel_1_r2: assert(nstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_selection_buf_reset_r1: assert(selection_buf == $past(selection));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_selection_buf_reset_r2: assert(selection_buf == $past(selection));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001 && $past(d_in_1[1]) == 1'b1)
        a_pstate_trans_001_to_0_r1: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(pstate) == 3'b001 && $past(d_in_0[1]) == 1'b1)
        a_pstate_trans_001_to_0_r2: assert(pstate == 3'b010);
end
