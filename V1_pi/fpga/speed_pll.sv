module speed_pll(input clk_in, output clk_out);
	parameter 	[31:0]	DIV = 32'd100_000;
	
	reg 		[32:0] 	counter;
	always_ff @(posedge clk_in)
		if (counter >= DIV - 1)
			counter <= 32'b0;
		else 
			counter <= counter + 1;

	assign clk_out = (counter < DIV/2);
endmodule