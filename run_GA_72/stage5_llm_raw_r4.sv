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
