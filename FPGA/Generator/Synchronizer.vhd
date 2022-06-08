----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    23:31:10 05/15/2020 
-- Design Name: 
-- Module Name:    Synchronizer - Behavioral 
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

entity Synchronizer is
	 Generic(stages : integer);
    Port ( CLK : in  STD_LOGIC;
           SYNC_IN : in  STD_LOGIC;
           SYNC_OUT : out  STD_LOGIC);
end Synchronizer;

architecture Behavioral of Synchronizer is
	signal sync_line : std_logic_vector(stages downto 0);
begin

	SYNC_OUT <= sync_line(stages);

	process(CLK)
	begin
		if rising_edge(CLK) then
			sync_line <= sync_line(stages-1 downto 0) & SYNC_IN;
		end if;
	end process;

end Behavioral;

