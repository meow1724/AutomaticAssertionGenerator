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
  reg [7:0] PI_d_in_0;
  reg [7:0] PI_d_in_1;
  wire [0:0] PI_clk = clock;
  reg [0:0] PI_rst;
  reg [0:0] PI_selection;
  reg [0:0] PI_enable;
  top UUT (
    .d_in_0(PI_d_in_0),
    .d_in_1(PI_d_in_1),
    .clk(PI_clk),
    .rst(PI_rst),
    .selection(PI_selection),
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
    // UUT.$auto$async2sync.\cc:107:execute$1639  = 1'b0;
    // UUT.$auto$async2sync.\cc:116:execute$1643  = 1'b1;
    UUT._witness_.anyinit_auto_proc_dlatch_cc_432_proc_dlatch_1295 = 1'b0;
    UUT._witness_.anyinit_procdff_1456 = 3'b000;
    UUT._witness_.anyinit_procdff_1457 = 1'b0;
    UUT._witness_.anyinit_procdff_1458 = 1'b1;
    UUT._witness_.anyinit_procdff_1627 = 3'b000;
    UUT.d_o = 1'b0;
    UUT.f_past_valid = 1'b0;
    UUT.fmsv_init_cnt = 2'b00;
    UUT.selection_buf = 1'b0;
    UUT.wr_en = 1'b0;

    // state 0
    PI_d_in_0 = 8'b00000001;
    PI_d_in_1 = 8'b00000000;
    PI_rst = 1'b0;
    PI_selection = 1'b0;
    PI_enable = 1'b0;
  end
  always @(posedge clock) begin
    // state 1
    if (cycle == 0) begin
      PI_d_in_0 <= 8'b00000001;
      PI_d_in_1 <= 8'b00000000;
      PI_rst <= 1'b0;
      PI_selection <= 1'b1;
      PI_enable <= 1'b0;
    end

    // state 2
    if (cycle == 1) begin
      PI_d_in_0 <= 8'b00000001;
      PI_d_in_1 <= 8'b00000000;
      PI_rst <= 1'b0;
      PI_selection <= 1'b0;
      PI_enable <= 1'b0;
    end

    genclock <= cycle < 2;
    cycle <= cycle + 1;
  end
endmodule
