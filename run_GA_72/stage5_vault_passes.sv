// Stage 5 vault — current-round BMC-passed ASSERTS (covers excluded)
always @(posedge clk) if (f_past_valid) a_d_o_tracks_d_o_reg: assert(d_o == $past(d_o_reg));
always @(posedge clk) if (f_past_valid) a_wr_en_tracks_wr_en_reg: assert(wr_en == $past(wr_en_reg));
always @(posedge clk) if (f_past_valid) a_pstate_stable: assert($stable(pstate) || (pstate != $past(pstate)));
always @(posedge clk) if (f_past_valid) a_nstate_stable: assert($stable(nstate) || (nstate != $past(nstate)));
always @(posedge clk) if (f_past_valid) a_selection_buf_tracks: assert(selection_buf == $past(selection));
always @(posedge clk) if (f_past_valid) a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
always @(posedge clk) if (f_past_valid) a_d_o_tracks: assert(d_o == $past(d_o_reg));
a_d_o_reset_r3: assert(d_o == $past(d_in_1[0]));
a_d_o_rel_r3: assert(d_o == $past(d_in_1[0]));
a_wr_en_reset_r3: assert(wr_en == 1'b1);
a_trans_001_to_010_r3: assert(pstate == 3'b010);
a_trans_010_to_011_r3: assert(pstate == 3'b011);
a_pstate_trans_001_to_010_r3: assert(pstate == 3'b010);
