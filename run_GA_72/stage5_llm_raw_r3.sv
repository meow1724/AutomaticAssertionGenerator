always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_reset_r3: assert(d_o == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_d_o_func_r3: assert(d_o == $past(d_in_0[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_rel_r3: assert(d_o == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_wr_en_reset_r3: assert(wr_en == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(d_in_1[0]) == 1'b1)
        a_trans_000_to_001_r3: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001 && $past(d_in_1[1]) == 1'b1)
        a_trans_001_to_010_r3: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b010 && $past(d_in_1[2]) == 1'b1)
        a_trans_010_to_011_r3: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_reg_reset_r3: assert(d_o_reg == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_wr_en_reg_reset_r3: assert(wr_en_reg == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_d_o_reg_tracks_r3: assert(d_o == d_o_reg);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        a_wr_en_reg_tracks_r3: assert(wr_en == wr_en_reg);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        a_nstate_reset_r3: assert(nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b000)
        a_pstate_trans_000_to_001_r3: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1 && $past(pstate) == 3'b001)
        a_pstate_trans_001_to_010_r3: assert(pstate == 3'b010);
end
