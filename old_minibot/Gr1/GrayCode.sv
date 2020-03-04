
//=======================================================
//  This code is generated by Terasic System Builder
//=======================================================

module GrayCode(

	//////////// CLOCK //////////
	CLOCK_50,

	//////////// LED //////////
	LED,

	//////////// KEY //////////
	KEY,

	//////////// SW //////////
	SW,

	//////////// SDRAM //////////
	DRAM_ADDR,
	DRAM_BA,
	DRAM_CAS_N,
	DRAM_CKE,
	DRAM_CLK,
	DRAM_CS_N,
	DRAM_DQ,
	DRAM_DQM,
	DRAM_RAS_N,
	DRAM_WE_N,

	//////////// EPCS //////////
	EPCS_ASDO,
	EPCS_DATA0,
	EPCS_DCLK,
	EPCS_NCSO,

	//////////// Accelerometer and EEPROM //////////
	G_SENSOR_CS_N,
	G_SENSOR_INT,
	I2C_SCLK,
	I2C_SDAT,

	//////////// ADC //////////
	ADC_CS_N,
	ADC_SADDR,
	ADC_SCLK,
	ADC_SDAT,

	//////////// 2x13 GPIO Header //////////
	GPIO_2,
	GPIO_2_IN,

	//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
	RPi,
	RPi_IN,

	//////////// GPIO_1, GPIO_1 connect to GPIO Default //////////
	GPIO,
	GPIO_IN 
);

	//=======================================================
	//  PARAMETER declarations
	//=======================================================


	//=======================================================
	//  PORT declarations
	//=======================================================

	//////////// CLOCK //////////
	input 		          		CLOCK_50;

	//////////// LED //////////
	output		     [7:0]		LED;

	//////////// KEY //////////
	input 		     [1:0]		KEY;

	//////////// SW //////////
	input 		     [3:0]		SW;

	//////////// SDRAM //////////
	output		    [12:0]		DRAM_ADDR;
	output		     [1:0]		DRAM_BA;
	output		          		DRAM_CAS_N;
	output		          		DRAM_CKE;
	output		          		DRAM_CLK;
	output		          		DRAM_CS_N;
	inout 		    [15:0]		DRAM_DQ;
	output		     [1:0]		DRAM_DQM;
	output		          		DRAM_RAS_N;
	output		          		DRAM_WE_N;

	//////////// EPCS //////////
	output		          		EPCS_ASDO;
	input 		          		EPCS_DATA0;
	output		          		EPCS_DCLK;
	output		          		EPCS_NCSO;

	//////////// Accelerometer and EEPROM //////////
	output		          		G_SENSOR_CS_N;
	input 		          		G_SENSOR_INT;
	output		          		I2C_SCLK;
	inout 		          		I2C_SDAT;

	//////////// ADC //////////
	output		          		ADC_CS_N;
	output		          		ADC_SADDR;
	output		          		ADC_SCLK;
	input 		          		ADC_SDAT;

	//////////// 2x13 GPIO Header //////////
	inout 		    [12:0]		GPIO_2;
	input 		     [2:0]		GPIO_2_IN;

	//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
	inout 		    [33:0]		RPi;
	input 		     [1:0]		RPi_IN;

	//////////// GPIO_1, GPIO_1 connect to GPIO Default //////////
	inout 		    [33:0]		GPIO;
	input 		     [1:0]		GPIO_IN;


	//=======================================================
	//  REG/WIRE declarations
	//=======================================================

	// Motor Encoders (x3)
	logic quadA_right, quadB_right, quadA_left, quadB_left, quadA_laser, quadB_laser;
	assign quadA_laser = GPIO[4];
	assign quadB_laser = GPIO[5];
	assign quadA_left = GPIO[0];
	assign quadB_left = GPIO_IN[0];
	assign quadA_right = GPIO[2];
	assign quadB_right = GPIO[1];

	// Laser detection
	logic signal_laser, sync_laser;
	assign signal_laser = GPIO[8];
	assign sync_laser   = GPIO[7];

	// Data from RPi
	logic 	[31:0]	data_out;


	//=======================================================
	//  Structural coding
	//=======================================================
	// Clocks
	logic TIME_CLOCK, PLL_CLOCK;
	speed_pll time_clock(CLOCK_50, TIME_CLOCK);		// cannot be used on flip-flops
	my_pll pll_clock(CLOCK_50, PLL_CLOCK);			// pll --> freq = 2 kHz


	// Encoder Counters
	logic [31:0] count_right, count_left, count_laser;
	quad encoder_right(CLOCK_50, 1'b0, quadA_right, quadB_right, count_right);
	quad encoder_left(CLOCK_50, 1'b0, quadA_left, quadB_left, count_left);
	quad encoder_laser(CLOCK_50, sync_laser, quadA_laser, quadB_laser, count_laser);

	// Delta Calculators
	logic [31:0] speed_right, speed_left, start_laser, end_laser;
	speed speed_counter_right(PLL_CLOCK, count_right, speed_right);
	speed speed_counter_left(PLL_CLOCK, count_left, speed_left);
	delta delta_counter_laser(CLOCK_50, signal_laser, count_laser, start_laser, end_laser);

	// Display counter on LEDs
	assign LED = data_out[7:0];

	// SPI
	logic 			spi_clk, spi_cs, spi_mosi, spi_miso;
	logic	[31:0]	data_write, data_read;
	logic 	[3:0]	data_addr;
	spi_slave spi_slave_instance(CLOCK_50, spi_clk, spi_cs, spi_mosi, spi_miso, 
								// DATA TO SEND TO RPi :
								speed_right, speed_left, start_laser, end_laser, count_right, count_left, count_laser,
								// DATA TO RECEIVE FROM RPi :
								data_out);

	assign spi_clk  		= RPi[11];	// SCLK = pin 16 = RPi_11
	assign spi_cs   		= RPi[9];	// CE0  = pin 14 = RPi_9
	assign spi_mosi     	= RPi[15];	// MOSI = pin 20 = RPi_15
	assign RPi[13]			= spi_miso;	// MISO = pin 18 = RPi_13 

endmodule



//=======================================================
//  Module declarations
//=======================================================

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
