always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_d_o_reset_r2: assert(d_o == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(selection) == 1'b0) 
        a_d_o_func_r2: assert(d_o == $past(d_in_0[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(selection) == 1'b1) 
        a_d_o_rel_r2: assert(d_o == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_wr_en_reset_r2: assert(wr_en == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1) 
        a_trans_000_to_001_r2: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(d_in_1[0]) == 1'b1) 
        a_trans_000_to_001_sel_r2: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b0) 
        a_trans_001_to_010_r2: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b1 && $past(d_in_1[1]) == 1'b0) 
        a_trans_001_to_010_sel_r2: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b010 && $past(selection) == 1'b0 && $past(d_in_0[2]) == 1'b0) 
        a_trans_010_to_011_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b010 && $past(selection) == 1'b1 && $past(d_in_1[2]) == 1'b0) 
        a_trans_010_to_011_sel_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_d_o_reg_reset_r2: assert(d_o_reg == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b0) 
        a_d_o_reg_func_r2: assert(d_o_reg == $past(d_in_0[0]));
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b1) 
        a_d_o_reg_rel_r2: assert(d_o_reg == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_wr_en_reg_reset_r2: assert(wr_en_reg == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b1) 
        a_wr_en_reg_func_r2: assert(wr_en_reg == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b0) 
        a_wr_en_reg_rel_r2: assert(wr_en_reg == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid) 
        a_d_o_reg_tracks_r2: assert(d_o == d_o_reg);
end

always @(posedge clk) begin
    if (f_past_valid) 
        a_wr_en_reg_tracks_r2: assert(wr_en == wr_en_reg);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b0) 
        a_pstate_trans_000_to_001_r2: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(selection) == 1'b1) 
        a_pstate_trans_000_to_001_sel_r2: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b000) 
        a_nstate_reset_r2: assert(nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b001) 
        a_nstate_trans_000_to_001_r2: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b0) 
        a_trans_001_to_010_r2: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b1) 
        a_trans_001_to_010_sel_r2: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b010 && $past(selection) == 1'b0) 
        a_trans_010_to_011_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b010 && $past(selection) == 1'b1) 
        a_trans_010_to_011_sel_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b011 && $past(selection) == 1'b0) 
        a_trans_011_to_110_r2: assert(pstate == 3'b110);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b011 && $past(selection) == 1'b1) 
        a_trans_011_to_110_sel_r2: assert(pstate == 3'b110);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b110 && $past(selection) == 1'b0) 
        a_trans_110_to_100_r2: assert(pstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b110 && $past(selection) == 1'b1) 
        a_trans_110_to_100_sel_r2: assert(pstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b100 && $past(selection) == 1'b0) 
        a_trans_100_to_000_r2: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b100 && $past(selection) == 1'b1) 
        a_trans_100_to_000_sel_r2: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b101 && $past(selection) == 1'b0) 
        a_trans_101_to_011_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b101 && $past(selection) == 1'b1) 
        a_trans_101_to_011_sel_r2: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b111 && $past(selection) == 1'b0) 
        a_trans_111_to_111_r2: assert(pstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable) && $past(pstate) == 3'b111 && $past(selection) == 1'b1) 
        a_trans_111_to_111_sel_r2: assert(pstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable)) 
        a_pstate_trans_r2: assert(pstate == 3'b000 || pstate == 3'b001 || pstate == 3'b010 || pstate == 3'b011 || pstate == 3'b100 || pstate == 3'b101 || pstate == 3'b110 || pstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && $past(enable)) 
        a_nstate_trans_r2: assert(nstate == 3'b000 || nstate == 3'b001 || nstate == 3'b010 || nstate == 3'b011 || nstate == 3'b100 || nstate == 3'b101 || nstate == 3'b110 || nstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid) 
        a_selection_buf_tracks_r2: assert(selection_buf == $past(selection));
end
