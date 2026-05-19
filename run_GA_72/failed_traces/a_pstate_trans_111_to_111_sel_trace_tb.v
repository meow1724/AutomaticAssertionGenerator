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
  wire [0:0] PI_clk = clock;
  reg [7:0] PI_d_in_1;
  reg [0:0] PI_selection;
  reg [0:0] PI_enable;
  reg [0:0] PI_rst;
  reg [7:0] PI_d_in_0;
  top UUT (
    .clk(PI_clk),
    .d_in_1(PI_d_in_1),
    .selection(PI_selection),
    .enable(PI_enable),
    .rst(PI_rst),
    .d_in_0(PI_d_in_0)
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
    // UUT.$auto$async2sync.\cc:107:execute$1800  = 1'b0;
    // UUT.$auto$async2sync.\cc:116:execute$1804  = 1'b1;
    UUT._witness_.anyinit_auto_proc_dlatch_cc_432_proc_dlatch_1428 = 1'b1;
    UUT._witness_.anyinit_procdff_1583 = 3'b000;
    UUT._witness_.anyinit_procdff_1584 = 1'b1;
    UUT._witness_.anyinit_procdff_1585 = 1'b0;
    UUT._witness_.anyinit_procdff_1788 = 3'b000;
    UUT.d_o = 1'b1;
    UUT.f_past_valid = 1'b0;
    UUT.fmsv_init_cnt = 2'b00;
    UUT.selection_buf = 1'b1;
    UUT.wr_en = 1'b1;

    // state 0
    PI_d_in_1 = 8'b00000000;
    PI_selection = 1'b0;
    PI_enable = 1'b1;
    PI_rst = 1'b0;
    PI_d_in_0 = 8'b00000000;
  end
  always @(posedge clock) begin
    // state 1
    if (cycle == 0) begin
      PI_d_in_1 <= 8'b00000001;
      PI_selection <= 1'b0;
      PI_enable <= 1'b1;
      PI_rst <= 1'b0;
      PI_d_in_0 <= 8'b00000000;
    end

    // state 2
    if (cycle == 1) begin
      PI_d_in_1 <= 8'b00000001;
      PI_selection <= 1'b1;
      PI_enable <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
    end

    // state 3
    if (cycle == 2) begin
      PI_d_in_1 <= 8'b01110101;
      PI_selection <= 1'b1;
      PI_enable <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b10001010;
    end

    // state 4
    if (cycle == 3) begin
      PI_d_in_1 <= 8'b10010010;
      PI_selection <= 1'b1;
      PI_enable <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b10100110;
    end

    // state 5
    if (cycle == 4) begin
      PI_d_in_1 <= 8'b11011011;
      PI_selection <= 1'b1;
      PI_enable <= 1'b0;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b10001011;
    end

    // state 6
    if (cycle == 5) begin
      PI_d_in_1 <= 8'b11111110;
      PI_selection <= 1'b1;
      PI_enable <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
    end

    // state 7
    if (cycle == 6) begin
      PI_d_in_1 <= 8'b00000001;
      PI_selection <= 1'b1;
      PI_enable <= 1'b1;
      PI_rst <= 1'b1;
      PI_d_in_0 <= 8'b00000000;
    end

    genclock <= cycle < 7;
    cycle <= cycle + 1;
  end
endmodule
