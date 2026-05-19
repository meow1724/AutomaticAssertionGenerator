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
  wire [0:0] PI_clk = clock;
  reg [0:0] PI_rst;
  reg [7:0] PI_d_in_1;
  reg [0:0] PI_enable;
  reg [0:0] PI_selection;
  top UUT (
    .d_in_0(PI_d_in_0),
    .clk(PI_clk),
    .rst(PI_rst),
    .d_in_1(PI_d_in_1),
    .enable(PI_enable),
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
    // UUT.$auto$async2sync.\cc:107:execute$594  = 1'b0;
    // UUT.$auto$async2sync.\cc:116:execute$598  = 1'b1;
    UUT._witness_.anyinit_auto_proc_dlatch_cc_432_proc_dlatch_445 = 1'b0;
    UUT._witness_.anyinit_procdff_578 = 1'b0;
    UUT._witness_.anyinit_procdff_583 = 3'b110;
    UUT.d_o = 1'b1;
    UUT.selection_buf = 1'b0;
    UUT.wr_en = 1'b1;

    // state 0
    PI_d_in_0 = 8'b00000000;
    PI_rst = 1'b1;
    PI_d_in_1 = 8'b01111111;
    PI_enable = 1'b0;
    PI_selection = 1'b1;
  end
  always @(posedge clock) begin
    // state 1
    if (cycle == 0) begin
      PI_d_in_0 <= 8'b00000000;
      PI_rst <= 1'b1;
      PI_d_in_1 <= 8'b01111111;
      PI_enable <= 1'b0;
      PI_selection <= 1'b1;
    end

    genclock <= cycle < 1;
    cycle <= cycle + 1;
  end
endmodule
