----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    16:01:43 05/05/2020 
-- Design Name: 
-- Module Name:    MCP33131 - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity MCP33131 is
	 Generic(CLK_DIV : integer;
				CONVCYCLES : integer);
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           START : in  STD_LOGIC;
           READY : out  STD_LOGIC;
           DATA : out  STD_LOGIC_VECTOR (15 downto 0);
			  MIN : out STD_LOGIC_VECTOR (15 downto 0);
			  MAX : out STD_LOGIC_VECTOR (15 downto 0);
			  RESET_MINMAX : in STD_LOGIC;
           SDO : in  STD_LOGIC;
           CONVSTART : out  STD_LOGIC;
           SCLK : inout  STD_LOGIC);
end MCP33131;

architecture Behavioral of MCP33131 is
	signal conv_cnt : integer range 0 to CONVCYCLES-1;
	signal div_cnt : integer range 0 to (CLK_DIV/2)-1;
	signal bit_cnt : integer range 0 to 15;
	signal sclk_phase : std_logic;
	signal adc_data : std_logic_vector(15 downto 0);
	type States is (Idle, Conversion, WAIT_tEN, Transmission, Done);
	signal state : States;
	signal ready_int : std_logic;
	signal ready_delay : integer range 0 to 30;
	signal min_int, max_int, data_int : signed(15 downto 0);
begin

	MIN <= std_logic_vector(min_int);
	MAX <= std_logic_vector(max_int);
	DATA <= std_logic_vector(data_int);
	SCLK <= sclk_phase;

	process(SCLK, START)
	begin
		if(falling_edge(SCLK)) then
			adc_data <= adc_data(14 downto 0) & SDO;
		end if;
	end process;

	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				state <= Idle;
				READY <= '0';
				ready_int <= '0';
				CONVSTART <= '0';
				sclk_phase <= '0';
				CONVSTART <= '0';
				conv_cnt <= 0;
				div_cnt <= 0;
				bit_cnt <= 0;
				min_int <= to_signed(32767, 16);
				max_int <= to_signed(-32768, 16);
			else
				if RESET_MINMAX = '1' then
					min_int <= to_signed(32767, 16);
					max_int <= to_signed(-32768, 16);
				else
					if data_int < min_int then
						min_int <= data_int;
					end if;
					if data_int > max_int then
						max_int <= data_int;
					end if;
				end if;
				
				READY <= '0';
				if ready_int = '1' then
					ready_delay <= 3;
				else
					if ready_delay > 0 then
						ready_delay <= ready_delay - 1;
					end if;
					if ready_delay = 1 then
						READY <= '1';
						data_int <= signed(adc_data);
					end if;
				end if;
				
				case state is
					when Idle =>
						ready_int <= '0';
						if START = '1' then
							state <= Conversion;
							conv_cnt <= 0;
							CONVSTART <= '1';
						end if;
					when Conversion =>
						ready_int <= '0';
						if(conv_cnt < CONVCYCLES-1) then
							conv_cnt <= conv_cnt + 1;
						else
							div_cnt <= 0;
							CONVSTART <= '0';
							state <= WAIT_tEN;
						end if;
					when WAIT_tEN =>
						ready_int <= '0';
						state <= Transmission;
					when Transmission =>
						ready_int <= '0';
						if(div_cnt < (CLK_DIV/2)-1) then
							div_cnt <= div_cnt + 1;
						else
							if(sclk_phase = '0') then
								sclk_phase <= '1';
							else
								sclk_phase <= '0';
								if bit_cnt = 15 then
									state <= Done;
									bit_cnt <= 0;
								else
									bit_cnt <= bit_cnt + 1;
								end if;
							end if;
							div_cnt <= 0;
						end if;
					when Done =>
						ready_int <= '1';
						state <= Idle;
				end case;
			end if;
		end if;
	end process;

end Behavioral;

