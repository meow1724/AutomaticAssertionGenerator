always @(posedge clk) begin
    if (f_past_valid && !$past(rst))
        a_nstate_reset_rf1: assert(nstate == 3'b000);
end
always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b0)
        a_nstate_trans_000_rf1: assert(nstate == 3'b000);
end
always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1)
        a_nstate_trans_001_rf1: assert(nstate == 3'b001);
end
always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b0)
        a_nstate_trans_011_rf1: assert(nstate == 3'b011);
end
always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b1)
        a_nstate_trans_010_rf1: assert(nstate == 3'b010);
end
