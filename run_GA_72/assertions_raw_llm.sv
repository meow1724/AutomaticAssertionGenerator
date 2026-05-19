// == Auto-generated formal assertions ==
// Inputs (not asserted): clk, rst, enable, selection, d_in_0, d_in_1
// Assertable signals: d_o, wr_en, d_o_reg, wr_en_reg, pstate, nstate, selection_buf

reg f_past_valid;
initial f_past_valid = 0;
always @(posedge clk) f_past_valid <= 1;
reg [1:0] fmsv_init_cnt;
initial fmsv_init_cnt = 0;
always @(posedge clk) if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;
always @(*) if (fmsv_init_cnt < 2'd2) assume(!rst);

// --- Batch 1 Leader ---
always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_d_o_reset: assert(d_o == 1'b0);
    if (f_past_valid && $past(selection) == 1'b0) 
        a_d_o_func: assert(d_o == $past(d_in_0[0]));
    if (f_past_valid && $past(selection) == 1'b1) 
        a_d_o_rel: assert(d_o == $past(d_in_1[0]));
    c_d_o_reach: cover(d_o == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_wr_en_reset: assert(wr_en == 1'b0);
    if (f_past_valid && $past(selection) == 1'b0) 
        a_wr_en_func: assert(wr_en == 1'b0);
    if (f_past_valid && $past(selection) == 1'b1) 
        a_wr_en_rel: assert(wr_en == 1'b1);
    c_wr_en_reach: cover(wr_en == 1'b1);
end


// --- Batch 1 Extender ---
always @(posedge clk) begin
    if (f_past_valid)
        a_d_o_tracks_d_o_reg: assert(d_o == $past(d_o_reg));
end

always @(posedge clk) begin
    if (f_past_valid)
        a_wr_en_tracks_wr_en_reg: assert(wr_en == $past(wr_en_reg));
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1)
        a_trans_000_to_001: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b1)
        a_trans_001_to_010: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(selection) == 1'b0 && $past(d_in_0[2]) == 1'b1)
        a_trans_010_to_011: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) == 1'b0)
        a_pstate_reset: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(selection) == 1'b0)
        a_wr_en_low_when_selection_low: assert(wr_en == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && $past(selection) == 1'b1)
        a_wr_en_high_when_selection_high: assert(wr_en == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1)
        a_d_o_tracks_d_in_1: assert(d_o == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b1)
        a_d_o_tracks_d_in_1_bit1: assert(d_o == $past(d_in_1[1]));
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(selection) == 1'b1)
        a_d_o_tracks_d_in_1_bit2: assert(d_o == $past(d_in_1[2]));
end


// --- Batch 2 Leader ---
always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_d_o_reg_reset: assert(d_o_reg == 1'b0);
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1) 
        a_d_o_reg_func: assert(d_o_reg == $past(d_in_1[0]));
    if (f_past_valid && $past(wr_en_reg) == 1'b1) 
        a_d_o_reg_rel: assert(d_o_reg == $past(d_o_reg));
    c_d_o_reg_reach: cover(d_o_reg == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_wr_en_reg_reset: assert(wr_en_reg == 1'b0);
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1) 
        a_wr_en_reg_func: assert(wr_en_reg == 1'b1);
    if (f_past_valid && $past(selection) == 1'b1) 
        a_wr_en_reg_rel: assert(wr_en_reg == $past(wr_en_reg));
    c_wr_en_reg_reach: cover(wr_en_reg == 1'b1);
end


// --- Batch 2 Extender ---
always @(posedge clk) begin
    if (f_past_valid)
        a_d_o_reg_tracks: assert(d_o == d_o_reg);
end

always @(posedge clk) begin
    if (f_past_valid)
        a_wr_en_reg_tracks: assert(wr_en == wr_en_reg);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1)
// [DEDUP] a_trans_000_to_001: assert(pstate == 3'b001);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b1)
// [DEDUP] a_trans_001_to_010: assert(pstate == 3'b010);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1 && $past(d_in_1[0]) == 1'b1)
        a_trans_000_to_001_sel: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b1 && $past(d_in_1[1]) == 1'b1)
        a_trans_001_to_010_sel: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(selection) == 1'b0)
        a_wr_en_reg_selection: assert(wr_en_reg == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && $past(selection) == 1'b1)
        a_wr_en_reg_selection_1: assert(wr_en_reg == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) == 1'b0)
// [DEDUP] a_pstate_reset: assert(pstate == 3'b000);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(enable) == 1'b0)
        a_pstate_enable: assert(pstate == 3'b000);
end


// --- Batch 3 Leader ---
always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_pstate_reset: assert(pstate == 3'b000);
        ;
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1) 
        a_pstate_trans_000_to_001: assert(pstate == 3'b001);
    if (f_past_valid) 
        a_pstate_stable: assert($stable(pstate) || (pstate != $past(pstate)));
    if (f_past_valid && pstate == 3'b000) 
        c_pstate_000: cover(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_nstate_reset: assert(nstate == 3'b000);
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1) 
        a_nstate_trans_000_to_001: assert(nstate == 3'b001);
    if (f_past_valid) 
        a_nstate_stable: assert($stable(nstate) || (nstate != $past(nstate)));
    if (f_past_valid && nstate == 3'b001) 
        c_nstate_001: cover(nstate == 3'b001);
end


// --- Batch 3 Extender ---
always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_pstate_reset: assert(pstate == 3'b000);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1)
// [DEDUP] a_trans_000_to_001: assert(pstate == 3'b001);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b1)
// [DEDUP] a_trans_001_to_010: assert(pstate == 3'b010);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(selection) == 1'b0 && $past(d_in_0[2]) == 1'b1)
// [DEDUP] a_trans_010_to_011: assert(pstate == 3'b011);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b011 && $past(selection) == 1'b0 && $past(d_in_0[3]) == 1'b1)
        a_trans_011_to_110: assert(pstate == 3'b110);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b110 && $past(selection) == 1'b0 && $past(d_in_0[6]) == 1'b1)
        a_trans_110_to_100: assert(pstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b100 && $past(selection) == 1'b0 && $past(d_in_0[4]) == 1'b1)
        a_trans_100_to_000: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b101 && $past(selection) == 1'b0 && $past(d_in_0[5]) == 1'b1)
        a_trans_101_to_011: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b111 && $past(selection) == 1'b0 && $past(d_in_0[7]) == 1'b1)
        a_trans_111_to_111: assert(pstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1 && $past(d_in_1[0]) == 1'b1)
// [DEDUP] a_trans_000_to_001_sel: assert(pstate == 3'b001);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b1 && $past(d_in_1[1]) == 1'b1)
// [DEDUP] a_trans_001_to_010_sel: assert(pstate == 3'b010);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(selection) == 1'b1 && $past(d_in_1[2]) == 1'b1)
        a_trans_010_to_011_sel: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b011 && $past(selection) == 1'b1 && $past(d_in_1[3]) == 1'b1)
        a_trans_011_to_110_sel: assert(pstate == 3'b110);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b110 && $past(selection) == 1'b1 && $past(d_in_1[6]) == 1'b1)
        a_trans_110_to_100_sel: assert(pstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b100 && $past(selection) == 1'b1 && $past(d_in_1[4]) == 1'b1)
        a_trans_100_to_000_sel: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b101 && $past(selection) == 1'b1 && $past(d_in_1[5]) == 1'b1)
        a_trans_101_to_011_sel: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b111 && $past(selection) == 1'b1 && $past(d_in_1[7]) == 1'b1)
        a_trans_111_to_111_sel: assert(pstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(rst) == 1'b0)
        a_pstate_reset_to_nstate: assert(nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(rst) == 1'b0)
        a_nstate_reset_to_pstate: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(rst) == 1'b0)
        a_nstate_reset_to_pstate_010: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b011 && $past(rst) == 1'b0)
        a_nstate_reset_to_pstate_011: assert(nstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b100 && $past(rst) == 1'b0)
        a_nstate_reset_to_pstate_100: assert(nstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b101 && $past(rst) == 1'b0)
        a_nstate_reset_to_pstate_101: assert(nstate == 3'b101);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b110 && $past(rst) == 1'b0)
        a_nstate_reset_to_pstate_110: assert(nstate == 3'b110);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b111 && $past(rst) == 1'b0)
        a_nstate_reset_to_pstate_111: assert(nstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0)
        a_nstate_000_sel_0: assert(nstate == 3'b000 || nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0)
        a_nstate_001_sel_0: assert(nstate == 3'b011 || nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(selection) == 1'b0)
        a_nstate_010_sel_0: assert(nstate == 3'b100 || nstate == 3'b101);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b011 && $past(selection) == 1'b0)
        a_nstate_011_sel_0: assert(nstate == 3'b111 || nstate == 3'b110);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b100 && $past(selection) == 1'b0)
        a_nstate_100_sel_0: assert(nstate == 3'b001 || nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b101 && $past(selection) == 1'b0)
        a_nstate_101_sel_0: assert(nstate == 3'b010 || nstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b110 && $past(selection) == 1'b0)
        a_nstate_110_sel_0: assert(nstate == 3'b101 || nstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b111 && $past(selection) == 1'b0)
        a_nstate_111_sel_0: assert(nstate == 3'b110 || nstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1)
        a_nstate_000_sel_1: assert(nstate == 3'b000 || nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b1)
        a_nstate_001_sel_1: assert(nstate == 3'b011 || nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(selection) == 1'b1)
        a_nstate_010_sel_1: assert(nstate == 3'b100 || nstate == 3'b101);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b011 && $past(selection) == 1'b1)
        a_nstate_011_sel_1: assert(nstate == 3'b111 || nstate == 3'b110);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b100 && $past(selection) == 1'b1)
        a_nstate_100_sel_1: assert(nstate == 3'b001 || nstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b101 && $past(selection) == 1'b1)
        a_nstate_101_sel_1: assert(nstate == 3'b010 || nstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b110 && $past(selection) == 1'b1)
        a_nstate_110_sel_1: assert(nstate == 3'b101 || nstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b111 && $past(selection) == 1'b1)
        a_nstate_111_sel_1: assert(nstate == 3'b110 || nstate == 3'b111);
end


// --- Batch 4 Leader ---
always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
        a_selection_buf_reset: assert(selection_buf == 1'b0);
    if (f_past_valid) 
        a_selection_buf_tracks: assert(selection_buf == $past(selection));
    if (f_past_valid && $past(selection_buf) != $past(selection)) 
        a_selection_buf_mismatch: assert(selection_buf == 1'b0);
    c_selection_buf_1: cover(selection_buf == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_pstate_reset: assert(pstate == 3'b000);
        ;
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1) 
// [DEDUP] a_pstate_trans_000_to_001: assert(pstate == 3'b001);
        ;
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b1) 
        a_pstate_trans_001_to_010: assert(pstate == 3'b010);
    c_pstate_001: cover(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_nstate_reset: assert(nstate == 3'b000);
        ;
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1) 
// [DEDUP] a_nstate_trans_000_to_001: assert(nstate == 3'b001);
        ;
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b1) 
        a_nstate_trans_001_to_010: assert(nstate == 3'b010);
// [DEDUP] c_nstate_001: cover(nstate == 3'b001);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_wr_en_reset: assert(wr_en == 1'b0);
        ;
    if (f_past_valid) 
        a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1) 
        a_wr_en_set: assert(wr_en == 1'b1);
    c_wr_en_1: cover(wr_en == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_wr_en_reg_reset: assert(wr_en_reg == 1'b0);
        ;
    if (f_past_valid) 
        a_wr_en_reg_tracks_2: assert(wr_en_reg == $past(wr_en));
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1) 
        a_wr_en_reg_set: assert(wr_en_reg == 1'b1);
    c_wr_en_reg_1: cover(wr_en_reg == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_d_o_reset: assert(d_o == 1'b0);
        ;
    if (f_past_valid) 
        a_d_o_tracks: assert(d_o == $past(d_o_reg));
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1) 
        a_d_o_set: assert(d_o == $past(d_in_1[0]));
    c_d_o_1: cover(d_o == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst)) 
// [DEDUP] a_d_o_reg_reset: assert(d_o_reg == 1'b0);
        ;
    if (f_past_valid) 
        a_d_o_reg_tracks_2: assert(d_o_reg == $past(d_o));
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1) 
        a_d_o_reg_set: assert(d_o_reg == $past(d_in_1[0]));
    c_d_o_reg_1: cover(d_o_reg == 1'b1);
end


// --- Batch 4 Extender ---
always @(posedge clk) begin
    if (f_past_valid) 
// [DEDUP] a_selection_buf_tracks: assert(selection_buf == $past(selection));
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b1)
// [DEDUP] a_trans_000_to_001: assert(pstate == 3'b001);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b1)
// [DEDUP] a_trans_001_to_010: assert(pstate == 3'b010);
        ;
end

always @(posedge clk) begin
    if (f_past_valid) 
// [DEDUP] a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
        ;
end

always @(posedge clk) begin
    if (f_past_valid) 
// [DEDUP] a_d_o_tracks: assert(d_o == $past(d_o_reg));
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b1)
        a_wr_en_reg_sel: assert(wr_en_reg == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) == 1'b0)
// [DEDUP] a_pstate_reset: assert(pstate == 3'b000);
        ;
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b000 && $past(selection) == 1'b0 && $past(d_in_0[0]) == 1'b0)
        a_stay_000: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b001 && $past(selection) == 1'b0 && $past(d_in_0[1]) == 1'b0)
        a_trans_001_to_011: assert(pstate == 3'b011);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b010 && $past(selection) == 1'b0 && $past(d_in_0[2]) == 1'b0)
        a_trans_010_to_100: assert(pstate == 3'b100);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b011 && $past(selection) == 1'b0 && $past(d_in_0[3]) == 1'b0)
        a_trans_011_to_111: assert(pstate == 3'b111);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b100 && $past(selection) == 1'b0 && $past(d_in_0[4]) == 1'b0)
        a_trans_100_to_001: assert(pstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b101 && $past(selection) == 1'b0 && $past(d_in_0[5]) == 1'b0)
        a_trans_101_to_010: assert(pstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b110 && $past(selection) == 1'b0 && $past(d_in_0[6]) == 1'b0)
        a_trans_110_to_101: assert(pstate == 3'b101);
end

always @(posedge clk) begin
    if (f_past_valid && $past(pstate) == 3'b111 && $past(selection) == 1'b0 && $past(d_in_0[7]) == 1'b0)
        a_trans_111_to_110: assert(pstate == 3'b110);
end

