// == Auto-generated formal assertions ==
// Inputs (not asserted): CLK_I, RST_I, ACK_I, DAT_I, ERR_I, RTY_I
// Assertable signals: ADR_O, CYC_O, DAT_O, SEL_O, STB_O, WE_O, GO, cycle_end, address, data, selects, write_flag, mem_sizes, write_burst_buffer, read_burst_buffer

reg f_past_valid;
initial f_past_valid = 0;
always @(posedge CLK_I) f_past_valid <= 1;
reg [1:0] fmsv_init_cnt;
initial fmsv_init_cnt = 0;
always @(posedge CLK_I) if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;
always @(*) if (fmsv_init_cnt < 2'd2) assume(RST_I);

// --- Batch 1 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_adr_o_reset: assert(ADR_O == 32'h0000_0000);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_adr_o_func: assert(ADR_O == address);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(address) == ADR_O) 
        a_adr_o_rel: assert(ADR_O == address);
end

always @(posedge CLK_I) 
    c_adr_o: cover(ADR_O == 32'h0000_0000);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_cyc_o_reset: assert(CYC_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_cyc_o_func: assert(CYC_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(CYC_O) == CYC_O) 
        a_cyc_o_rel: assert(CYC_O == $past(CYC_O));
end

always @(posedge CLK_I) 
    c_cyc_o: cover(CYC_O == 1'b1);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_stb_o_reset: assert(STB_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_stb_o_func: assert(STB_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(STB_O) == STB_O) 
        a_stb_o_rel: assert(STB_O == $past(STB_O));
end

always @(posedge CLK_I) 
    c_stb_o: cover(STB_O == 1'b1);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_we_o_reset: assert(WE_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag)) 
        a_we_o_func: assert(WE_O == write_flag);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(WE_O) == WE_O) 
        a_we_o_rel: assert(WE_O == $past(WE_O));
end

always @(posedge CLK_I) 
    c_we_o: cover(WE_O == 1'b1);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_sel_o_reset: assert(SEL_O == 4'b1111);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1'b0) 
        a_sel_o_func: assert(SEL_O == 4'b1111);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(SEL_O) == SEL_O) 
        a_sel_o_rel: assert(SEL_O == $past(SEL_O));
end

always @(posedge CLK_I) 
    c_sel_o: cover(SEL_O == 4'b1111);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_dat_o_reset: assert(DAT_O == 32'h0000_0000);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1'b0) 
        a_dat_o_func: assert(DAT_O == DAT_I);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(DAT_O) == DAT_O) 
        a_dat_o_rel: assert(DAT_O == $past(DAT_O));
end

always @(posedge CLK_I) 
    c_dat_o: cover(DAT_O == 32'h0000_0000);


// --- Batch 1 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_adr_o_reset: assert(ADR_O == 32'h0000_0000);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I) && $past(GO))
        a_adr_o_update: assert(ADR_O == $past(address));
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I) && $past(address) == 32'h0000_0000)
        a_adr_o_default: assert(ADR_O == 32'h0000_0000);
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_cyc_o_reset: assert(CYC_O == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I) && $past(GO))
        a_cyc_o_active: assert(CYC_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I) && $past(CYC_O))
        a_cyc_o_deactive: assert(CYC_O == 1'b0);
end


// --- Batch 2 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_DAT_O_reset: assert(DAT_O == 32'h0000_0000);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1) 
        a_DAT_O_func: assert(DAT_O == $past(data));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1) 
        a_DAT_O_rel: assert(DAT_O == { $past(data[7:0]), $past(data[7:0]), $past(data[7:0]), $past(data[7:0]) } || 
                         DAT_O == { $past(data[15:0]), $past(data[15:0]) } || 
                         DAT_O == $past(data));
end

always @(posedge CLK_I) 
    c_DAT_O: cover(DAT_O != 32'h0000_0000);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_SEL_O_reset: assert(SEL_O == 4'hF);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1) 
        a_SEL_O_func: assert(SEL_O == $past(selects));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1) 
        a_SEL_O_rel: assert(SEL_O == {3'b000, $past(selects[0])} || 
                            SEL_O == {2'b00, $past(selects[1:0])} || 
                            SEL_O == $past(selects));
end

always @(posedge CLK_I) 
    c_SEL_O: cover(SEL_O != 4'hF);


// --- Batch 2 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_DAT_O_reset: assert(DAT_O == 32'h0000_0000);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
        a_SEL_O_reset_2: assert(SEL_O == 4'b1111);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 0)
        a_SEL_O_default: assert(SEL_O == 4'b1111);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1 && $past(data_width(address)) == 2'b00)
        a_SEL_O_byte: assert(SEL_O == {3'b000, $past(selects[0])});
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1 && $past(data_width(address)) == 2'b01)
        a_SEL_O_half: assert(SEL_O == {2'b00, $past(selects[1:0])});
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1 && $past(data_width(address)) == 2'b10)
        a_SEL_O_word: assert(SEL_O == $past(selects));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1 && $past(data_width(address)) == 2'b00)
        a_DAT_O_byte: assert(DAT_O == { $past(data[7:0]), $past(data[7:0]), $past(data[7:0]), $past(data[7:0]) });
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1 && $past(data_width(address)) == 2'b01)
        a_DAT_O_half: assert(DAT_O == { $past(data[15:0]), $past(data[15:0]) });
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1 && $past(data_width(address)) == 2'b10)
        a_DAT_O_word: assert(DAT_O == $past(data));
end


// --- Batch 3 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
// [DEDUP] a_stb_o_reset: assert(STB_O == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && !$past(STB_O)) 
// [DEDUP] a_stb_o_func: assert(STB_O == 1'b1);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
// [DEDUP] a_stb_o_rel: assert(STB_O == $past(STB_O));
        ;
end

always @(posedge CLK_I) 
// [DEDUP] c_stb_o: cover(STB_O == 1'b1);
        ;

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
// [DEDUP] a_we_o_reset: assert(WE_O == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag)) 
        a_we_o_func_2: assert(WE_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_we_o_rel_2: assert(WE_O == $past(write_flag));
end

always @(posedge CLK_I) 
// [DEDUP] c_we_o: cover(WE_O == 1'b1);
        ;


// --- Batch 3 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_stb_o_reset: assert(STB_O == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO))
        a_stb_o_set: assert(STB_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(STB_O) && $past(ACK_I))
        a_stb_o_clear: assert(STB_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_we_o_reset: assert(WE_O == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO))
        a_we_o_set: assert(WE_O == $past(write_flag));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
        a_we_o_reset_after_rst: assert(WE_O == 1'b0);
end


// --- Batch 4 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_go_reset: assert(GO == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && GO) 
        a_go_func: assert(CYC_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && CYC_O) 
        a_go_rel: assert(GO == 1'b0);
end

always @(posedge CLK_I) 
    c_go_reach: cover(GO == 1'b1);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_cycle_end_reset: assert(cycle_end == 1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && GO) 
        a_cycle_end_func: assert(cycle_end == 1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && CYC_O) 
        a_cycle_end_rel: assert(cycle_end == 1);
end

always @(posedge CLK_I) 
    c_cycle_end_reach: cover(cycle_end == 1);


// --- Batch 4 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_go_reset: assert(GO == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && !$past(RST_I))
        a_go_clear: assert(GO == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
// [DEDUP] a_cycle_end_reset: assert(cycle_end == 1);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && !$past(RST_I) && $past(cycle_end) == 1)
        a_cycle_end_set: assert(cycle_end == 1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I) && $past(GO))
        a_go_cycle_end: assert(cycle_end == 1);
end


// --- Batch 5 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_address_reset: assert(address == 32'd0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 0) 
        a_address_tracks: assert(address == $past(address));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && $past(write_flag) == 0) 
        a_address_func: assert(address == $past(address));
end

always @(posedge CLK_I) 
    c_address_reachable: cover(address == 32'd0);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_data_reset: assert(data == 32'd0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1) 
        a_data_tracks: assert(data == $past(data));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && $past(write_flag) == 1) 
        a_data_func: assert(data == $past(data));
end

always @(posedge CLK_I) 
    c_data_reachable: cover(data == 32'd0);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_ADR_O_reset: assert(ADR_O == 32'd0);
end

always @(posedge CLK_I) begin
    if (f_past_valid) 
        a_ADR_O_tracks: assert(ADR_O == $past(address));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_ADR_O_func: assert(ADR_O == $past(address));
end

always @(posedge CLK_I) 
    c_ADR_O_reachable: cover(ADR_O == 32'd0);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_CYC_O_reset: assert(CYC_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_CYC_O_tracks: assert(CYC_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(CYC_O) && $past(ACK_I)) 
        a_CYC_O_func: assert(CYC_O == 1'b0);
end

always @(posedge CLK_I) 
    c_CYC_O_reachable: cover(CYC_O == 1'b1);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_STB_O_reset: assert(STB_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_STB_O_tracks: assert(STB_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(STB_O) && $past(ACK_I)) 
        a_STB_O_func: assert(STB_O == 1'b0);
end

always @(posedge CLK_I) 
    c_STB_O_reachable: cover(STB_O == 1'b1);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
// [DEDUP] a_SEL_O_reset: assert(SEL_O == 4'b1111);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 0) 
        a_SEL_O_tracks: assert(SEL_O == 4'b1111);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1) 
// [DEDUP] a_SEL_O_func: assert(SEL_O == $past(selects));
        ;
end

always @(posedge CLK_I) 
    c_SEL_O_reachable: cover(SEL_O == 4'b1111);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_WE_O_reset: assert(WE_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO)) 
        a_WE_O_tracks: assert(WE_O == $past(write_flag));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && $past(write_flag) == 1) 
        a_WE_O_func: assert(WE_O == 1'b1);
end

always @(posedge CLK_I) 
    c_WE_O_reachable: cover(WE_O == 1'b1);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_DAT_O_reset_2: assert(DAT_O == 32'd0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1) 
        a_DAT_O_tracks: assert(DAT_O == $past(data));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1 && $past(data_width(address)) == 2'b10) 
// [DEDUP] a_DAT_O_func: assert(DAT_O == $past(data));
        ;
end

always @(posedge CLK_I) 
    c_DAT_O_reachable: cover(DAT_O == 32'd0);


// --- Batch 5 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_address_reset: assert(address == 32'd0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid)
        a_address_tracks_2: assert(ADR_O == $past(address));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I) && $past(write_flag) == 0)
// [DEDUP] a_data_reset: assert(data == 32'd0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(STB_O) && $past(ACK_I) && $past(write_flag) == 0)
        a_data_tracks_2: assert(data == $past(DAT_I));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && $past(write_flag) == 1)
        a_data_write: assert(data == $past(data));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
// [DEDUP] a_WE_O_reset: assert(WE_O == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO))
// [DEDUP] a_WE_O_tracks: assert(WE_O == $past(write_flag));
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
// [DEDUP] a_CYC_O_reset: assert(CYC_O == 1'b0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && !$past(CYC_O))
        a_CYC_O_rise: assert(CYC_O == 1'b1);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(STB_O) && $past(ACK_I))
        a_STB_O_fall: assert(STB_O == 1'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && !$past(STB_O))
        a_STB_O_rise: assert(STB_O == 1'b1);
end


// --- Batch 6 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_selects_reset: assert(selects == 255);
end

always @(posedge CLK_I) begin
    if (f_past_valid && write_flag == 1) 
        a_selects_write: assert(selects == $past(selects));
end

always @(posedge CLK_I) begin
    if (f_past_valid && write_flag == 0) 
        a_selects_read: assert(selects == 255);
end

always @(posedge CLK_I) 
    c_selects_255: cover(selects == 255);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_write_flag_reset: assert(write_flag == 0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && write_flag == 1) 
        a_write_flag_write: assert(SEL_O == selects);
end

always @(posedge CLK_I) begin
    if (f_past_valid && write_flag == 0) 
        a_write_flag_read: assert(SEL_O == 4'b1111);
end

always @(posedge CLK_I) 
    c_write_flag_1: cover(write_flag == 1);


// --- Batch 6 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_selects_reset: assert(selects == 255);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 0)
        a_selects_tracks: assert(selects == 255);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(write_flag) == 1)
        a_selects_tracks_write: assert(selects == $past(selects));
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_write_flag_reset: assert(write_flag == 0);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && $past(write_flag) == 0)
        a_write_flag_tracks: assert(write_flag == 0);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && $past(write_flag) == 1)
        a_write_flag_tracks_write: assert(write_flag == 1);
end


// --- Batch 7 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_mem_sizes_reset: assert(mem_sizes == 16'b10_01_10_11_00_01_10_11);
end

always @(posedge CLK_I) begin
    if (f_past_valid) 
        a_mem_sizes_func: assert(mem_sizes == 16'b10_01_10_11_00_01_10_11);
end

always @(posedge CLK_I) begin
    if (f_past_valid) 
        a_mem_sizes_rel: assert(mem_sizes == {data_width(ADR_O[31:29]), 14'b0});
end

always @(posedge CLK_I) 
    c_mem_sizes: cover(mem_sizes == 16'b10_01_10_11_00_01_10_11);

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_write_burst_buffer_reset: assert(write_burst_buffer[0] == 32'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid) 
        a_write_burst_buffer_func: assert(write_burst_buffer[0] == 32'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid) 
        a_write_burst_buffer_rel: assert(write_burst_buffer[0] == DAT_O);
end

always @(posedge CLK_I) 
    c_write_burst_buffer: cover(write_burst_buffer[0] == 32'b0);


// --- Batch 7 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid)
        a_mem_sizes_stable: assert(mem_sizes == $past(mem_sizes));
end

always @(posedge CLK_I) begin
    if (f_past_valid)
        a_write_burst_buffer_stable: assert(write_burst_buffer[0] == $past(write_burst_buffer[0]));
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
        a_mem_sizes_unchanged: assert(mem_sizes == $past(mem_sizes));
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
        a_write_burst_buffer_unchanged: assert(write_burst_buffer[1] == $past(write_burst_buffer[1]));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
        a_write_burst_buffer_reset_2: assert(write_burst_buffer[2] == 32'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid)
        a_mem_sizes_value: assert(mem_sizes == 16'b10_01_10_11_00_01_10_11);
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
        a_write_burst_buffer_track: assert(write_burst_buffer[3] == $past(write_burst_buffer[3]));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
// [DEDUP] a_mem_sizes_reset: assert(mem_sizes == 16'b10_01_10_11_00_01_10_11);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid)
        a_write_burst_buffer_track_2: assert(write_burst_buffer[4] == $past(write_burst_buffer[4]));
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
        a_mem_sizes_no_change: assert(mem_sizes == $past(mem_sizes));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
        a_write_burst_buffer_reset_2: assert(write_burst_buffer[5] == 32'b0);
end

always @(posedge CLK_I) begin
    if (f_past_valid)
        a_write_burst_buffer_track_3: assert(write_burst_buffer[6] == $past(write_burst_buffer[6]));
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
        a_mem_sizes_same: assert(mem_sizes == $past(mem_sizes));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
        a_write_burst_buffer_reset_3: assert(write_burst_buffer[7] == 32'b0);
end


// --- Batch 8 Leader ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I)) 
        a_read_burst_buffer_reset: assert(read_burst_buffer[0] == 32'h0000_0000);
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && !$past(write_flag)) 
        a_read_burst_buffer_func: assert(read_burst_buffer[0] == $past(DAT_I));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && $past(write_flag)) 
        a_read_burst_buffer_rel: assert(read_burst_buffer[0] == $past(data));
end

always @(posedge CLK_I) c_read_burst_buffer: cover(read_burst_buffer[0] == 32'h0000_0000);


// --- Batch 8 Extender ---
always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I))
// [DEDUP] a_read_burst_buffer_reset: assert(read_burst_buffer[0] == 32'h0000_0000);
        ;
end

always @(posedge CLK_I) begin
    if (f_past_valid)
        a_read_burst_buffer_tracks: assert(read_burst_buffer[0] == $past(read_burst_buffer[0]));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(GO) && !$past(write_flag))
        a_read_burst_buffer_update: assert(read_burst_buffer[0] == $past(DAT_I));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(RST_I))
        a_read_burst_buffer_rst: assert(read_burst_buffer[0] == 32'h0000_0000);
end

always @(posedge CLK_I) begin
    if (f_past_valid && !$past(RST_I) && $past(STB_O) && $past(ACK_I))
        a_read_burst_buffer_stable: assert(read_burst_buffer[0] == $past(read_burst_buffer[0]));
end

always @(posedge CLK_I) begin
    if (f_past_valid && $past(cycle_end) && !$past(write_flag))
        a_read_burst_buffer_cycle_end: assert(read_burst_buffer[0] == $past(DAT_I));
end

