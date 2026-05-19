`ifndef VERILATOR
module testbench;
  reg [4095:0] vcdfile;
  reg clock;
`else
module testbench(input clock, output reg genclock);
  initial genclock = 1;
`endif
  reg genclock = 1;
  reg [31:0] cycle = 0;
  reg [0:0] PI_selection;
  reg [0:0] PI_rst;
  wire [0:0] PI_clk = clock;
  reg [7:0] PI_d_in_0;
  reg [7:0] PI_d_in_1;
  reg [0:0] PI_enable;
  top UUT (
    .selection(PI_selection),
    .rst(PI_rst),
    .clk(PI_clk),
    .d_in_0(PI_d_in_0),
    .d_in_1(PI_d_in_1),
    .enable(PI_enable)
  );
`ifndef VERILATOR
  initial begin
    if ($value$plusargs("vcd=%s", vcdfile)) begin
      $dumpfile(vcdfile);
      $dumpvars(0, testbench);
    end
    #5 clock = 0;
    while (genclock) begin
      #5 clock = 0;
      #5 clock = 1;
    end
  end
`endif
  initial begin
`ifndef VERILATOR
    #1;
`endif
    // UUT.$auto$async2sync.\cc:107:execute$1775  = 1'b0;
    // UUT.$auto$async2sync.\cc:116:execute$1779  = 1'b1;
    UUT._witness_.anyinit_auto_proc_dlatch_cc_432_proc_dlatch_1409 = 1'b0;
    UUT._witness_.anyinit_procdff_1594 = 3'b000;
    UUT._witness_.anyinit_procdff_1595 = 1'b1;
    UUT._witness_.anyinit_procdff_1763 = 3'b000;
    UUT.d_o = 1'b1;
    UUT.f_past_valid = 1'b0;
    UUT.fmsv_init_cnt = 2'b00;
    UUT.selection_buf = 1'b1;
    UUT.wr_en = 1'b1;

    // state 0
    PI_selection = 1'b1;
    PI_rst = 1'b0;
    PI_d_in_0 = 8'b00000000;
    PI_d_in_1 = 8'b00000001;
    PI_enable = 1'b1;
  end
  always @(posedge clock) begin
    // state 1
    if (cycle == 0) begin
      PI_selection <= 1'b0;
      PI_rst <= 1'b0;
      PI_d_in_0 <= 8'b00000000;
      PI_d_in_1 <= 8'b00000000;
      PI_enable <= 1'b1;
    end

    // state 2
    if (cycle == 1) begin
      PI_selection <= 1'b0;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b00000001;
      PI_d_in_1 <= 8'b00000000;
      PI_enable <= 1'b1;
    end

    // state 3
    if (cycle == 2) begin
      PI_selection <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b10001010;
      PI_d_in_1 <= 8'b01100101;
      PI_enable <= 1'b1;
    end

    // state 4
    if (cycle == 3) begin
      PI_selection <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b10001010;
      PI_d_in_1 <= 8'b00011101;
      PI_enable <= 1'b1;
    end

    // state 5
    if (cycle == 4) begin
      PI_selection <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b01000000;
      PI_d_in_1 <= 8'b10111111;
      PI_enable <= 1'b1;
    end

    // state 6
    if (cycle == 5) begin
      PI_selection <= 1'b1;
      PI_rst <= 1'b0;
      PI_d_in_0 <= 8'b00000000;
      PI_d_in_1 <= 8'b11111110;
      PI_enable <= 1'b0;
    end

    // state 7
    if (cycle == 6) begin
      PI_selection <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
      PI_d_in_1 <= 8'b00000000;
      PI_enable <= 1'b1;
    end

    genclock <= cycle < 7;
    cycle <= cycle + 1;
  end
endmodule
