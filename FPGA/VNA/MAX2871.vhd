----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    16:59:45 05/05/2020 
-- Design Name: 
-- Module Name:    MAX2871 - Behavioral 
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
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity MAX2871 is
	 Generic (CLK_DIV : integer);
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           REG4 : in  STD_LOGIC_VECTOR (31 downto 0);
           REG3 : in  STD_LOGIC_VECTOR (31 downto 0);
           REG1 : in  STD_LOGIC_VECTOR (31 downto 0);
           REG0 : in  STD_LOGIC_VECTOR (31 downto 0);
           RELOAD : in  STD_LOGIC;
           CLK_OUT : out  STD_LOGIC;
           MOSI : out  STD_LOGIC;
           LE : out  STD_LOGIC;
           DONE : out  STD_LOGIC);
end MAX2871;

architecture Behavioral of MAX2871 is
	signal clk_cnt : integer range 0 to (CLK_DIV/2)-1;
	signal reg_cnt : integer range 0 to 3;
	signal bit_cnt : integer range 0 to 32;
	signal latched_regs : std_logic_vector(127 downto 0);
	
	signal sclk : std_logic;
	signal latch : std_logic;
	signal done_int : std_logic;
begin

	CLK_OUT <= sclk;
	MOSI <= latched_regs(127);
	LE <= latch;
	DONE <= done_int;

	process(CLK, RESET)
	begin
		if rising_edge(CLK) then
			if RESET = '1' then
				sclk <= '0';
				latch <= '0';
				done_int <= '1';
			else
				if done_int = '1' then
					-- can start a new reload process
					if RELOAD = '1' then
						done_int <= '0';
						latched_regs <= REG4 & REG3 & REG1 & REG0;
						reg_cnt <= 0;
						bit_cnt <= 0;
						clk_cnt <= 0;
					end if;
				else
					if clk_cnt < (CLK_DIV/2) - 1 then
						clk_cnt <= clk_cnt + 1;
					else
						clk_cnt <= 0;
						-- advance SPI state machine
						if bit_cnt < 32 then
							if sclk = '0' then
								sclk <= '1';
							else
								-- falling edge of clk, shift out new bit
								sclk <= '0';
								latched_regs <= latched_regs(126 downto 0) & "0";
								bit_cnt <= bit_cnt + 1;
							end if;
						else
							-- shifted out one register, strobe latch
							if latch = '0' then
								latch <= '1';
							else
								latch <= '0';
								-- move on to next register
								if reg_cnt < 3 then
									reg_cnt <= reg_cnt + 1;
									bit_cnt <= 0;
								else
									-- all done
									done_int <= '1';
								end if;
							end if;
						end if;
					end if;
				end if;
			end if;
		end if;
	end process;
end Behavioral;

