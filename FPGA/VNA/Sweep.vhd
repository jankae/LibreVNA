----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    21:35:02 05/06/2020 
-- Design Name: 
-- Module Name:    Sweep - Behavioral 
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

entity Sweep is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
			  NPOINTS : in STD_LOGIC_VECTOR (12 downto 0);
           CONFIG_ADDRESS : out  STD_LOGIC_VECTOR (12 downto 0);
           CONFIG_DATA : in  STD_LOGIC_VECTOR (95 downto 0);
			  USER_NSAMPLES : in STD_LOGIC_VECTOR (12 downto 0);
			  NSAMPLES : out STD_LOGIC_VECTOR (12 downto 0);
			  SETTLING_TIME : in STD_LOGIC_VECTOR (19 downto 0);
           SAMPLING_BUSY : in STD_LOGIC;
			  SAMPLING_DONE : in  STD_LOGIC;
			  START_SAMPLING : out STD_LOGIC;
			  BAND_SELECT : out STD_LOGIC;
			  -- fixed part of source/LO registers
           MAX2871_DEF_4 : in  STD_LOGIC_VECTOR (31 downto 0);
           MAX2871_DEF_3 : in  STD_LOGIC_VECTOR (31 downto 0);
           MAX2871_DEF_1 : in  STD_LOGIC_VECTOR (31 downto 0);
           MAX2871_DEF_0 : in  STD_LOGIC_VECTOR (31 downto 0);
			  -- assembled source/LO registers
			  SOURCE_REG_4 : out  STD_LOGIC_VECTOR (31 downto 0);
           SOURCE_REG_3 : out  STD_LOGIC_VECTOR (31 downto 0);
           SOURCE_REG_1 : out  STD_LOGIC_VECTOR (31 downto 0);
           SOURCE_REG_0 : out  STD_LOGIC_VECTOR (31 downto 0);
			  LO_REG_4 : out  STD_LOGIC_VECTOR (31 downto 0);
           LO_REG_3 : out  STD_LOGIC_VECTOR (31 downto 0);
           LO_REG_1 : out  STD_LOGIC_VECTOR (31 downto 0);
           LO_REG_0 : out  STD_LOGIC_VECTOR (31 downto 0);
			  RELOAD_PLL_REGS : out STD_LOGIC;
			  PLL_RELOAD_DONE : in STD_LOGIC;
			  PLL_LOCKED : in STD_LOGIC;
			  SWEEP_HALTED : out STD_LOGIC;
			  SWEEP_RESUME : in STD_LOGIC;
			  
			  SYNC_ENABLED : in STD_LOGIC;
			  SYNC_MASTER : in STD_LOGIC;
			  TRIGGER_IN : in STD_LOGIC;
			  TRIGGER_OUT : out STD_LOGIC;
			  
			  NEW_DATA : out STD_LOGIC;
			  
			  ATTENUATOR : out STD_LOGIC_VECTOR(6 downto 0);
			  SOURCE_FILTER : out STD_LOGIC_VECTOR(1 downto 0);
			  
			  --SETTLING_TIME : in STD_LOGIC_VECTOR (15 downto 0);
			  
			  STAGES : in STD_LOGIC_VECTOR (2 downto 0);
			  PORT1_STAGE : in STD_LOGIC_VECTOR (2 downto 0);
			  PORT2_STAGE : in STD_LOGIC_VECTOR (2 downto 0);
			  
			  PORT1_ACTIVE : out STD_LOGIC;
			  PORT2_ACTIVE : out STD_LOGIC;
			  
			  SOURCE_CE : out STD_LOGIC;
			  
			  -- Debug signals
			  DEBUG_STATUS : out STD_LOGIC_VECTOR (10 downto 0);
			  RESULT_INDEX : out STD_LOGIC_VECTOR (15 downto 0)
			  );
end Sweep;

architecture Behavioral of Sweep is
	signal point_cnt : unsigned(12 downto 0);
	type Point_states is (WaitInitialLow, TriggerSetup, SettingUp, Settling, WaitTriggerHigh, Exciting, WaitTriggerLow, SamplingDone, NextPoint, Done);
	signal state : Point_states;
	signal settling_cnt : unsigned(19 downto 0);
	signal stage_cnt : unsigned (2 downto 0);
	signal config_reg : std_logic_vector(95 downto 0);
	signal source_active : std_logic;
begin
	
	CONFIG_ADDRESS <= std_logic_vector(point_cnt);
	
	-- assemble registers
	-- source register 0: N divider and fractional division value
	SOURCE_REG_0 <= MAX2871_DEF_0(31) & "000000000" & config_reg(93) & config_reg(5 downto 0) & config_reg(26 downto 15) & "000";
	-- source register 1: Modulus value
	SOURCE_REG_1 <= MAX2871_DEF_1(31 downto 15) & config_reg(38 downto 27) & "001";
	-- source register 3: VCO selection
	SOURCE_REG_3 <= config_reg(11 downto 6) & MAX2871_DEF_3(25 downto 3) & "011";
	-- output power A from config, output B disabled
	SOURCE_REG_4 <= MAX2871_DEF_4(31 downto 23) & config_reg(14 downto 12) & MAX2871_DEF_4(19 downto 9) & "000" & MAX2871_DEF_4(5) & config_reg(47 downto 46) & "100";
	
	-- LO register 0: N divider and fractional division value
	LO_REG_0 <= MAX2871_DEF_0(31) & "000000000" & config_reg(94) & config_reg(54 downto 49) & config_reg(75 downto 64) & "000";
	-- LO register 1: Modulus value
	LO_REG_1 <= MAX2871_DEF_1(31 downto 15) & config_reg(87 downto 76) & "001";
	-- LO register 3: VCO selection
	LO_REG_3 <= config_reg(60 downto 55) & MAX2871_DEF_3(25 downto 3) & "011";
	-- both outputs enabled at +5dbm
	LO_REG_4 <= MAX2871_DEF_4(31 downto 23) & config_reg(63 downto 61) & MAX2871_DEF_4(19 downto 9) & "111111100";
	
	ATTENUATOR <= config_reg(45 downto 39);
	SOURCE_FILTER <= config_reg(89 downto 88);
	BAND_SELECT <= config_reg(48);
	SOURCE_CE <= source_active;
							
	NSAMPLES <= USER_NSAMPLES when config_reg(92 downto 90) = "000" else
					std_logic_vector(to_unsigned(6, 13)) when config_reg(92 downto 90) = "001" else
					std_logic_vector(to_unsigned(19, 13)) when config_reg(92 downto 90) = "010" else
					std_logic_vector(to_unsigned(57, 13)) when config_reg(92 downto 90) = "011" else
					std_logic_vector(to_unsigned(190, 13)) when config_reg(92 downto 90) = "100" else
					std_logic_vector(to_unsigned(571, 13)) when config_reg(92 downto 90) = "101" else
					std_logic_vector(to_unsigned(1904, 13)) when config_reg(92 downto 90) = "110" else
					std_logic_vector(to_unsigned(5712, 13));
	
	DEBUG_STATUS(10 downto 7) <= "0000" when state = TriggerSetup else
											"0001" when state = SettingUp else
											"0010" when state = Settling else
											"0011" when state = WaitTriggerHigh else
											"0100" when state = Exciting else
											"0101" when state = WaitTriggerLow else 
											"0110" when state = SamplingDone else
											"0111" when state = NextPoint else
											"1000" when state = Done else
											"1001";
	DEBUG_STATUS(6) <= PLL_RELOAD_DONE;
	DEBUG_STATUS(5) <= PLL_RELOAD_DONE and PLL_LOCKED;
	DEBUG_STATUS(4) <= SAMPLING_BUSY;
	DEBUG_STATUS(3) <= TRIGGER_IN;
	DEBUG_STATUS(2) <= source_active;
	DEBUG_STATUS(1 downto 0) <= (others => '1');

	config_reg <= CONFIG_DATA;

	process(CLK, RESET)
	begin
		if rising_edge(CLK) then
			if RESET = '1' then
				point_cnt <= (others => '0');
				stage_cnt <= (others => '0');
				state <= WaitInitialLow;
				START_SAMPLING <= '0';
				RELOAD_PLL_REGS <= '0';
				SWEEP_HALTED <= '0';
				RESULT_INDEX <= (others => '1');
				PORT1_ACTIVE <= '0';
				PORT2_ACTIVE <= '0';
				TRIGGER_OUT <= '0';
				source_active <= '0';
			else
				case state is
					when WaitInitialLow =>
						TRIGGER_OUT <= '0';
						if TRIGGER_IN = '0' or SYNC_ENABLED = '0' then
							state <= TriggerSetup;
						end if;
					when TriggerSetup =>
						RELOAD_PLL_REGS <= '1';
						if PLL_RELOAD_DONE = '0' then
							state <= SettingUp;
						end if;
					when SettingUp =>
						-- highest bit in config_reg determines whether the sweep should be halted prior to sampling
						SWEEP_HALTED <= config_reg(95);
						RELOAD_PLL_REGS <= '0';
						settling_cnt <= unsigned(SETTLING_TIME);
						if PLL_RELOAD_DONE = '1' and PLL_LOCKED = '1' then
							-- check if halted sweep is resumed
							if config_reg(95) = '0' or SWEEP_RESUME = '1' then
								SWEEP_HALTED <= '0';
   							state <= Settling;
							end if;
						end if;
					when Settling =>
						NEW_DATA <= '0';
						source_active <= '0';
						if std_logic_vector(stage_cnt) = PORT1_STAGE then
							PORT1_ACTIVE <= '1';
							source_active <= '1';
						else
							PORT1_ACTIVE <= '0';
						end if;
						if std_logic_vector(stage_cnt) = PORT2_STAGE then
							PORT2_ACTIVE <= '1';
							source_active <= '1';
						else
							PORT2_ACTIVE <= '0';
						end if;
						-- wait for settling time to elapse
						if settling_cnt > 0 then
							settling_cnt <= settling_cnt - 1;
						else
							-- need to wait for the trigger
							state <= WaitTriggerHigh;
							if SYNC_MASTER = '1' then
								-- this device generates the stimulus signal, it needs start the trigger itself
								TRIGGER_OUT <= '1';
							end if;
						end if;
					when WaitTriggerHigh =>
						if TRIGGER_IN = '1' or SYNC_ENABLED = '0' then 
							TRIGGER_OUT <= SYNC_ENABLED; -- pass on trigger signal if enabled
							START_SAMPLING <= '1';
							if SAMPLING_BUSY = '1' then
								state <= Exciting;
							end if;
						end if;
					when Exciting =>
						-- wait for sampling to finish
						START_SAMPLING <= '0';
						if SAMPLING_BUSY = '0' then
							RESULT_INDEX <= std_logic_vector(stage_cnt) & std_logic_vector(point_cnt);
							state <= WaitTriggerLow;
						end if;
					when WaitTriggerLow =>
						NEW_DATA <= '0';
						if SYNC_MASTER = '1' then
							TRIGGER_OUT <= '0';
						end if;
						if TRIGGER_IN = '0' or SYNC_ENABLED = '0' then
							TRIGGER_OUT <= '0';
							state <= SamplingDone;
						end if;
					when SamplingDone =>
						NEW_DATA <= '1';
						if stage_cnt < unsigned(STAGES) then
							stage_cnt <= stage_cnt + 1;
							-- can go directly to preperation for next stage
							state <= Settling;
						else
							state <= NextPoint;
						end if;
						settling_cnt <= unsigned(SETTLING_TIME);
					when NextPoint =>
						NEW_DATA <= '0';
						if point_cnt < unsigned(NPOINTS) then
							point_cnt <= point_cnt + 1;
							stage_cnt <= (others => '0');
							state <= TriggerSetup;
						else 
							point_cnt <= (others => '0');
							state <= Done;
							TRIGGER_OUT <= '0';
						end if;					
					when others =>
				end case;
			end if;
		end if;
	end process;
end Behavioral;

