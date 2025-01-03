----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    22:14:17 03/05/2019 
-- Design Name: 
-- Module Name:    spi_slave - Behavioral 
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
use IEEE.numeric_std.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity spi_slave is
	generic ( W : integer;
				PREWIDTH : integer);
    Port ( SPI_CLK : in  STD_LOGIC;
           MISO : out  STD_LOGIC;
           MOSI : in  STD_LOGIC;
           CS : in  STD_LOGIC;
			  BUF_OUT : out STD_LOGIC_VECTOR (W-1 downto 0) := (others => '0');
			  BUF_IN : in STD_LOGIC_VECTOR (W-1 downto 0);
           CLK : in  STD_LOGIC;
			  COMPLETE : out STD_LOGIC;
			  -- processing the complete word after it is complete leaves very little time
			  -- for read operations. Indicate when the first PREWIDTH bits are ready which
			  -- allows more time to prepare the response to the next word
			  PRE_COMPLETE : out STD_LOGIC;
			  PRE_BUF_OUT : out STD_LOGIC_VECTOR (PREWIDTH-1 downto 0) := (others => '0')
				);
end spi_slave;

architecture Behavioral of spi_slave is
	signal miso_buffer : STD_LOGIC_VECTOR (W-1 downto 0);
	signal mosi_buffer : STD_LOGIC_VECTOR (W-2 downto 0);

	signal data_valid : STD_LOGIC_VECTOR(2 downto 0);
	signal data_synced : STD_LOGIC_VECTOR(2 downto 0);
	signal data : STD_LOGIC_VECTOR(W-1 downto 0);
	signal pre_data : STD_LOGIC_VECTOR(PREWIDTH-1 downto 0);
	signal pre_data_valid : STD_LOGIC_VECTOR(3 downto 0);
	
	signal bit_cnt : integer range 0 to W-1;
begin

	process(CLK)
	begin
		if rising_edge(CLK) then
			COMPLETE <= '0';
			PRE_COMPLETE <= '0';
			data_valid(2 downto 1) <= data_valid(1 downto 0);
			pre_data_valid(3 downto 1) <= pre_data_valid(2 downto 0);
			if data_valid(2) = '1' then
				if data_synced(0) = '0' then
					BUF_OUT <= data;
					COMPLETE <= '1';
					data_synced(0) <= '1';
				end if;
			else
				data_synced(0) <= '0';
			end if;
			if pre_data_valid(3 downto 2) = "01" then
				-- pre_data has just become valid
				PRE_BUF_OUT <= pre_data;
				PRE_COMPLETE <= '1';
			end if;
		end if;
	end process;

	MISO <= BUF_IN(15) when bit_cnt = 0 else miso_buffer(W-2);

	slave_in: process(SPI_CLK, CS)
	begin
		if CS = '1' then
			bit_cnt <= 0;
			data_valid(0) <= '0';
			pre_data_valid(0) <= '0';
		elsif rising_edge(SPI_CLK) then
			-- data input process: sample on the rising edge
			data_synced(2 downto 1) <= data_synced(1 downto 0);
			if bit_cnt = PREWIDTH-1 then
				-- first couple of bits are ready
				pre_data <= mosi_buffer(PREWIDTH-2 downto 0) & MOSI;
				pre_data_valid(0) <= '1';
			end if;
			if bit_cnt = W-1 then
				-- this was the last bit
				pre_data_valid(0) <= '0';
				data_valid(0) <= '1';
				data <= mosi_buffer(W-2 downto 0) & MOSI;
			else
				if data_valid(0) = '1' and data_synced(2) = '1' then
					data_valid(0) <= '0';
				end if;
				mosi_buffer <= mosi_buffer(W-3 downto 0) & MOSI;
			end if;
			
			-- data output process: data should be launched on the falling edge
			-- but the delay is too large. Launch on the rising edge instead
			if bit_cnt < W-1 then
				bit_cnt <= bit_cnt + 1;
				if bit_cnt = 0 then
					miso_buffer <= BUF_IN;
				else
					miso_buffer <= miso_buffer(W-2 downto 0) & '0';
				end if;
			else
				bit_cnt <= 0;
			end if;
		end if;
	end process;
	
--	slave_out: process(SPI_CLK, CS, BUF_IN, bit_cnt)
--	begin
--		if CS = '1' then
--			bit_cnt <= 0;
--		elsif falling_edge(SPI_CLK) then
--			if bit_cnt < W-1 then
--				bit_cnt <= bit_cnt + 1;
--				if bit_cnt = 0 then
--					miso_buffer <= BUF_IN;
--				else
--					miso_buffer <= miso_buffer(W-2 downto 0) & '0';
--				end if;
--			else
--				bit_cnt <= 0;
--				--miso_buffer <= BUF_IN;
--			end if;
--		end if;
--	end process;

end Behavioral;