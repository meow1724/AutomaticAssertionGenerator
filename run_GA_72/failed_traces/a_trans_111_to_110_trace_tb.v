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
  reg [7:0] PI_d_in_1;
  reg [0:0] PI_enable;
  reg [7:0] PI_d_in_0;
  wire [0:0] PI_clk = clock;
  reg [0:0] PI_rst;
  reg [0:0] PI_selection;
  top UUT (
    .d_in_1(PI_d_in_1),
    .enable(PI_enable),
    .d_in_0(PI_d_in_0),
    .clk(PI_clk),
    .rst(PI_rst),
    .selection(PI_selection)
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
    // UUT.$auto$async2sync.\cc:107:execute$1773  = 1'b0;
    // UUT.$auto$async2sync.\cc:116:execute$1777  = 1'b1;
    UUT._witness_.anyinit_auto_proc_dlatch_cc_432_proc_dlatch_1407 = 1'b0;
    UUT._witness_.anyinit_procdff_1558 = 3'b000;
    UUT._witness_.anyinit_procdff_1559 = 1'b0;
    UUT._witness_.anyinit_procdff_1560 = 1'b0;
    UUT._witness_.anyinit_procdff_1761 = 3'b000;
    UUT.d_o = 1'b0;
    UUT.f_past_valid = 1'b0;
    UUT.fmsv_init_cnt = 2'b00;
    UUT.selection_buf = 1'b0;
    UUT.wr_en = 1'b0;

    // state 0
    PI_d_in_1 = 8'b00000001;
    PI_enable = 1'b1;
    PI_d_in_0 = 8'b00000000;
    PI_rst = 1'b0;
    PI_selection = 1'b0;
  end
  always @(posedge clock) begin
    // state 1
    if (cycle == 0) begin
      PI_d_in_1 <= 8'b00000001;
      PI_enable <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
      PI_rst <= 1'b0;
      PI_selection <= 1'b0;
    end

    // state 2
    if (cycle == 1) begin
      PI_d_in_1 <= 8'b00000001;
      PI_enable <= 1'b1;
      PI_d_in_0 <= 8'b00000001;
      PI_rst <= 1'b1;
      PI_selection <= 1'b0;
    end

    // state 3
    if (cycle == 2) begin
      PI_d_in_1 <= 8'b10100010;
      PI_enable <= 1'b1;
      PI_d_in_0 <= 8'b11001101;
      PI_rst <= 1'b1;
      PI_selection <= 1'b0;
    end

    // state 4
    if (cycle == 3) begin
      PI_d_in_1 <= 8'b11111110;
      PI_enable <= 1'b1;
      PI_d_in_0 <= 8'b00000001;
      PI_rst <= 1'b1;
      PI_selection <= 1'b0;
    end

    // state 5
    if (cycle == 4) begin
      PI_d_in_1 <= 8'b11110110;
      PI_enable <= 1'b0;
      PI_d_in_0 <= 8'b01001000;
      PI_rst <= 1'b1;
      PI_selection <= 1'b0;
    end

    // state 6
    if (cycle == 5) begin
      PI_d_in_1 <= 8'b01111110;
      PI_enable <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
      PI_rst <= 1'b1;
      PI_selection <= 1'b1;
    end

    // state 7
    if (cycle == 6) begin
      PI_d_in_1 <= 8'b00000001;
      PI_enable <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
      PI_rst <= 1'b1;
      PI_selection <= 1'b0;
    end

    genclock <= cycle < 7;
    cycle <= cycle + 1;
  end
endmodule
