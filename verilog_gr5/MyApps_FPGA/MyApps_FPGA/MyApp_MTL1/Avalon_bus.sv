//-------------------------------------------------------------
// Avalon Bus Interface Module
//
// In this example, we paint a small rectangle
//
//-------------------------------------------------------------

module Avalon_bus(
	input  logic		iCLK,
	input  logic		iRST_n,
	input  logic		iPaint,	
		
	input  logic 		avl_waitrequest,                 
	output logic [31:0]	avl_address,                      
	input  logic 		avl_readdatavalid,                 
	input  logic [31:0]	avl_readdata,                      
	output logic [31:0]	avl_writedata,                     
	output logic 		avl_read,                          
	output logic 		avl_write,    
	output logic 		avl_burstbegin,
	output logic [4:0]	avl_burstcount		
);

`define RECT_X0		150
`define RECT_Y0		150
`define RECT_W		200
`define RECT_H		150

	assign  avl_writedata   = 32'h00FFE4C4;	// our color 
	assign  avl_burstcount  = 1;			// in this example, we do not implement burst transaction

//--- FSM to control Avalon Bus Interface ---------------------

	typedef enum logic [1:0] {S0,S1,S2,S3} statetype;
	statetype avl_state;
	
	logic [9:0]	cnt_row;
	logic [9:0]	cnt_column;
	
	assign avl_write = (avl_state == S1 && iRST_n == 1)?1:0;
	assign avl_read  = 0;

	always @ (posedge iCLK or negedge iRST_n) begin
	
		if (!iRST_n) begin
	    	avl_state <= S0;
		end
		else begin 
			case(avl_state)
	  
				S0 : if (!iPaint) begin
						avl_state <= S1;
						avl_address <= 32'h20000000 + (800 * 4 * `RECT_Y0) + (`RECT_X0 * 4);
	    				cnt_row <= 0;
	    				cnt_column <= 0;
	    			end
			
				S1 : begin
			
	      			if (!avl_waitrequest) begin
	      				if (cnt_column != `RECT_W) begin
	      					cnt_column <= cnt_column + 1;
	      					avl_address <= avl_address + 4;
	      				end else begin
	      					if (cnt_row != `RECT_H) begin
	      						cnt_column <= 0;
	      						cnt_row <= cnt_row + 1;
	      						avl_address <= avl_address + ((800 - `RECT_W) * 4);
	      					end
	      					else avl_state <= S2;
	      				end 
	      			end
					end
	      	
	      	S2 : if (iPaint) avl_state <= S0;
		 
			endcase
		end
	end

endmodule 
