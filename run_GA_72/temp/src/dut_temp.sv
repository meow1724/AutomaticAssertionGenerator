 
module top
(
   clk,
   rst,
   enable,
   selection,
   d_in_0,
   d_in_1,
   d_o,
   wr_en
);
   input       clk;
   input       rst;
   input       enable;
   input       selection;
   input [7:0] d_in_0;
   input [7:0] d_in_1;
   output reg  d_o;
   output reg  wr_en;
   reg         d_o_reg;
   reg         wr_en_reg;
   reg   [2:0] pstate;
   reg   [2:0] nstate;
   reg         selection_buf;
   always @(posedge clk)
   begin
      selection_buf  <= selection;
      wr_en          <= wr_en_reg;
      d_o            <= d_o_reg;
   end
   always @(posedge clk or negedge rst)
   begin
      if(rst==1'b0)
         pstate   <= 3'b000;
      else if(enable==1'b0)
         pstate   <= 3'b000;
      else if(selection_buf==1'b1 && selection==1'b0)
         pstate   <= 3'b000;
      else
         pstate   <= nstate;
   end
   always @(*)
   begin
      case (pstate)
         3'b000:
         begin
            if(selection==1'b0)
            begin
               wr_en_reg =  1'b0;
               case(d_in_0[0])
                  1'b0: nstate   =  3'b000;
                  1'b1: nstate   =  3'b001;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[0];  
               wr_en_reg =  1'b1;
               case(d_in_1[0])
                  1'b0: nstate   =  3'b000;
                  1'b1: nstate   =  3'b001;
               endcase
           end
         end
         3'b001:
         begin
            if(selection==1'b0)
             begin
             wr_en_reg =  1'b0;
               case(d_in_0[1])
                  1'b0: nstate   =  3'b011;
                  1'b1: nstate   =  3'b010;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[1];  
               wr_en_reg =  1'b1;
               case(d_in_1[1])
                  1'b0: nstate   =  3'b011;
                  1'b1: nstate   =  3'b010;
               endcase
           end
         end
         3'b010:
         begin
            if(selection==1'b0)
            begin
               wr_en_reg =  1'b0;
               case(d_in_0[2])
                  1'b0: nstate   =  3'b100;
                  1'b1: nstate   =  3'b101;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[2];  
               wr_en_reg =  1'b1;
               case(d_in_1[2])
                  1'b0: nstate   =  3'b100;
                  1'b1: nstate   =  3'b101;
               endcase
            end
         end
         3'b011:
         begin
            if(selection==1'b0)
            begin
               wr_en_reg =  1'b0;
               case(d_in_0[3])
                  1'b0: nstate   =  3'b111;
                  1'b1: nstate   =  3'b110;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[3]; 
               wr_en_reg =  1'b1;
               case(d_in_1[3])
                  1'b0: nstate   =  3'b111;
                  1'b1: nstate   =  3'b110;
               endcase
            end
         end
         3'b100:
         begin
            if(selection==1'b0)
            begin
               wr_en_reg =  1'b0;
               case(d_in_0[4])
                  1'b0: nstate   =  3'b001;
                  1'b1: nstate   =  3'b000;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[4];  
               wr_en_reg =  1'b1;
               case(d_in_1[4])
                  1'b0: nstate   =  3'b001;
                  1'b1: nstate   =  3'b000;
               endcase
            end
         end
         3'b101:
         begin
            if(selection==1'b0)
            begin
               wr_en_reg =  1'b0;
               case(d_in_0[5])
                  1'b0: nstate   =  3'b010;
                  1'b1: nstate   =  3'b011;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[5];  
               wr_en_reg =  1'b1;
               case(d_in_1[5])
                  1'b0: nstate   =  3'b010;
                  1'b1: nstate   =  3'b011;
               endcase
            end
         end
         3'b110:
         begin
            if(selection==1'b0)
            begin
               wr_en_reg =  1'b0;
               case(d_in_0[6])
                  1'b0: nstate   =  3'b101;
                  1'b1: nstate   =  3'b100;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[6];  
               wr_en_reg =  1'b1;
               case(d_in_1[6])
                  1'b0: nstate   =  3'b101;
                  1'b1: nstate   =  3'b100;
               endcase
            end
         end
         3'b111:
         begin
            if(selection==1'b0)
            begin
               wr_en_reg =  1'b0;
               case(d_in_0[7])
                  1'b0: nstate   =  3'b110;
                  1'b1: nstate   =  3'b111;
               endcase
            end
            else
            begin
               d_o_reg   =  d_in_1[7];  
               wr_en_reg =  1'b1;
               case(d_in_1[7])
                  1'b0: nstate   =  3'b110;
                  1'b1: nstate   =  3'b111;
               endcase
            end
         end
      endcase
   end

// --- INJECTED FMSV ASSERTIONS ---
// ---- Stage 5 formal preamble (regenerated every round) ----
reg f_past_valid;
initial f_past_valid = 0;
always @(posedge clk) f_past_valid <= 1;
reg [1:0] fmsv_init_cnt;
initial fmsv_init_cnt = 0;
always @(posedge clk) if (fmsv_init_cnt < 2'd3) fmsv_init_cnt <= fmsv_init_cnt + 1;
always @(*) if (fmsv_init_cnt < 2'd2) assume(!rst);

        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_d_o_reach: cover(d_o == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_wr_en_reach: cover(wr_en == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_d_o_tracks_d_o_reg: assert(d_o == $past(d_o_reg));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_wr_en_tracks_wr_en_reg: assert(wr_en == $past(wr_en_reg));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_d_o_reg_reach: cover(d_o_reg == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_wr_en_reg_reach: cover(wr_en_reg == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_pstate_stable: assert($stable(pstate) || (pstate != $past(pstate)));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_pstate_000: cover(pstate == 3'b000);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_nstate_stable: assert($stable(nstate) || (nstate != $past(nstate)));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_nstate_001: cover(nstate == 3'b001);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_selection_buf_tracks: assert(selection_buf == $past(selection));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_selection_buf_1: cover(selection_buf == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_pstate_001: cover(pstate == 3'b001);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_wr_en_tracks: assert(wr_en == $past(wr_en_reg));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_wr_en_1: cover(wr_en == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_wr_en_reg_1: cover(wr_en_reg == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_d_o_tracks: assert(d_o == $past(d_o_reg));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_d_o_1: cover(d_o == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) c_d_o_reg_1: cover(d_o_reg == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_d_o_reset_r3: assert(d_o == $past(d_in_1[0]));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_d_o_rel_r3: assert(d_o == $past(d_in_1[0]));
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_wr_en_reset_r3: assert(wr_en == 1'b1);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_trans_001_to_010_r3: assert(pstate == 3'b010);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_trans_010_to_011_r3: assert(pstate == 3'b011);
        begin end // [ISOLATED] always @(posedge clk) if (f_past_valid) a_pstate_trans_001_to_010_r3: assert(pstate == 3'b010);

// ---- Stage 5 round 4 refinements ----
always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        begin end // [ISOLATED] a_wr_en_func_r4: assert(wr_en == 1'b1);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        begin end // [ISOLATED] a_wr_en_low_when_selection_low_r4: assert(wr_en == 1'b0);
end

always @(posedge clk) begin
    if (f_past_valid && $past(rst) && !$past(enable))
        begin end // [ISOLATED] a_pstate_reset_r4: assert(pstate == 3'b000);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b1)
        begin end // [ISOLATED] a_d_o_tracks_d_in_1_r4: assert(d_o == $past(d_in_1[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(selection) == 1'b0)
        begin end // [ISOLATED] a_d_o_tracks_d_in_0_r4: assert(d_o == $past(d_in_0[0]));
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b000 && $past(selection) == 1'b1)
        begin end // [ISOLATED] a_nstate_trans_000_to_001_r4: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b1)
        begin end // [ISOLATED] a_nstate_trans_001_to_010_r4: assert(nstate == 3'b010);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b000 && $past(selection) == 1'b0)
        begin end // [ISOLATED] a_nstate_trans_000_to_001_r4_alt: assert(nstate == 3'b001);
end

always @(posedge clk) begin
    if (f_past_valid && !$past(rst) && $past(enable) && $past(pstate) == 3'b001 && $past(selection) == 1'b0)
        a_nstate_trans_001_to_010_r4_alt: assert(nstate == 3'b010);
end

endmodule

