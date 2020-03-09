module MyFirstDynamixel(
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

	//////////// 2x13 GPIO Header //////////
	GPIO_2,
	GPIO_2_IN,

	//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
	GPIO_0,
	GPIO_0_IN,

	//////////// GPIO_1, GPIO_1 connect to GPIO Default //////////
	GPIO_1,
	GPIO_1_IN 
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

//////////// 2x13 GPIO Header //////////
inout 		    [12:0]		GPIO_2;
input 		     [2:0]		GPIO_2_IN;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
inout 		    [33:0]		GPIO_0;
input 		     [1:0]		GPIO_0_IN;

//////////// GPIO_1, GPIO_1 connect to GPIO Default //////////
inout 		    [33:0]		GPIO_1;
input 		     [1:0]		GPIO_1_IN;


//=======================================================
//  SPI
//=======================================================

	logic spi_clk, spi_cs, spi_mosi, spi_miso, cs_spi, clk;
	logic [31:0] DataAdrM;
	logic [31:0] ReadData;
	logic [31:0] data1, data2;
	logic [2:0] enable;
	logic [2:0] rw_ad;
//	spi_slave spi_slave_instance(
//		.sck(spi_clk),
//		.mosi(spi_mosi),
//		.miso(spi_miso),
//		.reset(),
//		.d(DataToPI),
//		.q(DataFromPI)
//	);
//
//	assign spi_clk  		= GPIO_0[11];	// SCLK = pin 16 = GPIO_11
//	assign spi_cs   		= GPIO_0[9];	// CE0  = pin 14 = GPIO_9
//	assign spi_mosi     	= GPIO_0[15];	// MOSI = pin 20 = GPIO_15
//	
//	assign GPIO_0[13] = spi_cs ? 1'bz : spi_miso;  // MISO = pin 18 = GPIO_13									// 0x-0-- to 0x-3--
//	assign cs_data1    = ~DataAdrM[11] &  DataAdrM[10] & ~DataAdrM[9] & ~DataAdrM[8];	// 0x-4--
//	assign cs_led    = ~DataAdrM[11] &  DataAdrM[10] & ~DataAdrM[9] &  DataAdrM[8];	// 0x-5--
	
	// Read Data
//	always_comb begin 
////		if (cs_spi) ReadDataM = spi_data;
////		else ReadDataM = 32'b0;	
//		
//		ReadDataM 
//		end
	assign DataAdrM = 32'h03;

	spi_slave spi_slave_instance(
		.SPI_CLK    (spi_clk),
		.SPI_CS     (spi_cs),
		.SPI_MOSI   (spi_mosi),
		.SPI_MISO   (spi_miso),
		.Data_WE    (enable[1]),
		.Data_Addr  (DataAdrM),
		.Data_Write (ReadData),
		.Data_Read  (spi_data),
		.Clk        (CLOCK_50),
		.data1		(data1),
		.data2		(data2), 
		.rw_ad		(rw_ad),
		.enable		(enable),
	);
	
	assign spi_clk  		= GPIO_0[11];	// SCLK = pin 16 = GPIO_11
	assign spi_cs   		= GPIO_0[9];	// CE0  = pin 14 = GPIO_9
	assign spi_mosi     	= GPIO_0[15];	// MOSI = pin 20 = GPIO_15
	
	assign GPIO_0[13] = spi_cs ? 1'bz : spi_miso;  // MISO = pin 18 = GPIO_13
	

//always_comb begin
//    	ReadData = DataFromPI; 				// 0x400
//end

//=======================================================
//  Structural coding
//=======================================================
logic TXD, RXD, Direction_Port;

//logic read_en, write_en;

assign GPIO_1[22]  = Direction_Port;
assign GPIO_1[26]  = TXD;
assign RXD 			= GPIO_1[24];
assign DRAM_CLK   = CLOCK_50;
//assign read_en 	= 1'b0;
//assign write_en	= 1'b1;


UART_Dynamixel MyDyna(
	// NIOS signals
	.clk(CLOCK_50),
	.reset(~KEY[0]),
	.write_en(enable[0] & SW[0]),
	.read_en(enable[1] & SW[0]),
	.rw_ad(rw_ad),
	.write_data(),
	.read_data(ReadData),
	// exported signals
	.RXD(RXD),
	.TXD(TXD), 
	.UART_DIR(Direction_Port),
	.debug(),
	.LED(LED),
	.data1		(data1),
	.data2		(data2),
	.start(enable[2])
);

endmodule
