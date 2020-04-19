
// Quadrature Encoder Counter
module quad(clk, reset, quadA, quadB, count);
	input reset, clk, quadA, quadB;
	output [31:0] count;

	reg [2:0] quadA_delayed, quadB_delayed;
	always_ff @(posedge clk) quadA_delayed <= {quadA_delayed[1:0], quadA};
	always_ff @(posedge clk) quadB_delayed <= {quadB_delayed[1:0], quadB};

	wire count_enable = quadA_delayed[1] ^ quadA_delayed[2] ^ quadB_delayed[1] ^ quadB_delayed[2];
	wire count_direction = quadA_delayed[1] ^ quadB_delayed[2];

	always_ff @(posedge clk, posedge reset) begin
		if (reset)	count <= 32'b0;
		else if (count_enable)
			if (count_direction)	count <= count + 1; 
			else 					count <= count - 1;
	end
endmodule

// Speed Counter
module speed(clk, counter, speed);
	input 			clk;
	input 	[31:0] 	counter;
	output 	[31:0]	speed;

	reg 	[31:0] 	old_value, new_value;
	always_ff @(posedge clk) begin
		old_value <= new_value;
		new_value <= counter;
	end

	assign speed = new_value - old_value;
endmodule

// Delta Counter
module delta(clk, signal, counter, start, _end);
	input			clk;
	input 			signal;
	input 	[31:0] 	counter;
	output 	[31:0]	start, _end; 

	reg [2:0] 	resync;
	reg [31:0]	fall_value, rise_value, decr;

	always_ff @(posedge clk) begin
		resync <= {signal, resync[2:1]};

		if (decr > 32'b0)
			decr <= decr - 1;
		if (resync[1] & !resync[0])			// Posedge detected
			rise_value <= counter;
		if (resync[0] & !resync[1]) begin	// Negedge detected
			decr <= 32'd4_000_000;
			if (decr == 32'b0)
				fall_value <= counter;
		end
	end
	
	assign start = fall_value;
	assign _end  = rise_value;
endmodule
