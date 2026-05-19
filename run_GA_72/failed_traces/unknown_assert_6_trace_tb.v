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
  reg [0:0] PI_enable;
  reg [0:0] PI_rst;
  wire [0:0] PI_clk = clock;
  reg [7:0] PI_d_in_0;
  reg [7:0] PI_d_in_1;
  reg [0:0] PI_selection;
  top UUT (
    .enable(PI_enable),
    .rst(PI_rst),
    .clk(PI_clk),
    .d_in_0(PI_d_in_0),
    .d_in_1(PI_d_in_1),
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
    // UUT.$auto$async2sync.\cc:107:execute$591  = 1'b0;
    // UUT.$auto$async2sync.\cc:116:execute$595  = 1'b1;
    UUT._witness_.anyinit_auto_proc_dlatch_cc_432_proc_dlatch_443 = 1'b0;
    UUT._witness_.anyinit_procdff_580 = 3'b111;
    UUT.d_o = 1'b1;
    UUT.selection_buf = 1'b0;
    UUT.wr_en = 1'b1;

    // state 0
    PI_enable = 1'b0;
    PI_rst = 1'b1;
    PI_d_in_0 = 8'b00000000;
    PI_d_in_1 = 8'b11111111;
    PI_selection = 1'b1;
  end
  always @(posedge clock) begin
    // state 1
    if (cycle == 0) begin
      PI_enable <= 1'b0;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
      PI_d_in_1 <= 8'b11111111;
      PI_selection <= 1'b1;
    end

    genclock <= cycle < 1;
    cycle <= cycle + 1;
  end
endmodule
