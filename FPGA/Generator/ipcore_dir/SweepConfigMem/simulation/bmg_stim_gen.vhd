      


--------------------------------------------------------------------------------
--
-- BLK MEM GEN v7_3 Core - Stimulus Generator For Simple Dual Port RAM
--
--------------------------------------------------------------------------------
--
-- (c) Copyright 2006_3010 Xilinx, Inc. All rights reserved.
--
-- This file contains confidential and proprietary information
-- of Xilinx, Inc. and is protected under U.S. and
-- international copyright and other intellectual property
-- laws.
--
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- Xilinx, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) Xilinx shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or Xilinx had been advised of the
-- possibility of the same.
--
-- CRITICAL APPLICATIONS
-- Xilinx products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of Xilinx products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
--
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.

--------------------------------------------------------------------------------
--
-- Filename: bmg_stim_gen.vhd
--
-- Description:
--  Stimulus Generation For SDP Configuration
--  100 Writes and 100 Reads will be performed in a repeatitive loop till the 
--  simulation ends
--
--------------------------------------------------------------------------------
-- Author: IP Solutions Division
--
-- History: Sep 12, 2011 - First Release
--------------------------------------------------------------------------------
--
--------------------------------------------------------------------------------
-- Library Declarations
--------------------------------------------------------------------------------
LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;
USE IEEE.STD_LOGIC_MISC.ALL;

 LIBRARY work;
USE work.ALL;
USE work.BMG_TB_PKG.ALL;


ENTITY REGISTER_LOGIC IS
  PORT(
    Q   : OUT STD_LOGIC;
    CLK : IN STD_LOGIC;
    RST : IN STD_LOGIC;
    D   : IN STD_LOGIC
  );
END REGISTER_LOGIC;

ARCHITECTURE REGISTER_ARCH OF REGISTER_LOGIC IS
SIGNAL Q_O : STD_LOGIC :='0';
BEGIN
  Q <= Q_O;
  FF_BEH: PROCESS(CLK)
  BEGIN
     IF(RISING_EDGE(CLK)) THEN
        IF(RST ='1') THEN
           Q_O <= '0';
        ELSE
           Q_O <= D;
        END IF;
     END IF;
  END PROCESS;
END REGISTER_ARCH;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;
USE IEEE.STD_LOGIC_MISC.ALL;

LIBRARY work;
USE work.ALL;
USE work.BMG_TB_PKG.ALL;


ENTITY BMG_STIM_GEN IS
      PORT (
            CLKA : IN STD_LOGIC;
            CLKB : IN STD_LOGIC;
            TB_RST : IN STD_LOGIC;
            ADDRA: OUT  STD_LOGIC_VECTOR(12 DOWNTO 0) := (OTHERS => '0'); 
            DINA : OUT  STD_LOGIC_VECTOR(95 DOWNTO 0) := (OTHERS => '0'); 
            ENA : OUT STD_LOGIC :='0';
            WEA : OUT STD_LOGIC_VECTOR (0 DOWNTO 0) := (OTHERS => '0');
            ADDRB: OUT  STD_LOGIC_VECTOR(12 DOWNTO 0) := (OTHERS => '0');
	        CHECK_DATA: OUT STD_LOGIC:='0'
	  );
END BMG_STIM_GEN;


ARCHITECTURE BEHAVIORAL OF BMG_STIM_GEN IS

CONSTANT ZERO                    : STD_LOGIC_VECTOR(31 DOWNTO 0) := (OTHERS => '0');
SIGNAL   WRITE_ADDR              : STD_LOGIC_VECTOR(31 DOWNTO 0) := (OTHERS => '0');
SIGNAL   READ_ADDR               : STD_LOGIC_VECTOR(31 DOWNTO 0) := (OTHERS => '0');
SIGNAL   DINA_INT                : STD_LOGIC_VECTOR(95 DOWNTO 0) := (OTHERS => '0');
SIGNAL   DO_WRITE                : STD_LOGIC := '0';
SIGNAL   DO_READ                 : STD_LOGIC := '0';
SIGNAL   DO_READ_R               : STD_LOGIC := '0';
SIGNAL   DO_READ_REG             : STD_LOGIC_VECTOR(5 DOWNTO 0) :=(OTHERS => '0');
SIGNAL   PORTA_WR                : STD_LOGIC:='0';
SIGNAL   COUNT                   : INTEGER :=0;
SIGNAL   INCR_WR_CNT             : STD_LOGIC:='0';
SIGNAL   PORTA_WR_COMPLETE       : STD_LOGIC :='0';
SIGNAL   PORTB_RD                : STD_LOGIC:='0';
SIGNAL   COUNT_RD                : INTEGER :=0;
SIGNAL   INCR_RD_CNT             : STD_LOGIC:='0';
SIGNAL   PORTB_RD_COMPLETE       : STD_LOGIC :='0';
SIGNAL   LATCH_PORTA_WR_COMPLETE : STD_LOGIC :='0';
SIGNAL   PORTB_RD_HAPPENED       : STD_LOGIC := '0';
SIGNAL   PORTA_WR_L1             :STD_LOGIC  := '0';
SIGNAL   PORTA_WR_L2             :STD_LOGIC  := '0';
SIGNAL   PORTB_RD_R2             :STD_LOGIC  := '0';
SIGNAL   PORTB_RD_R1             :STD_LOGIC  := '0';
SIGNAL   LATCH_PORTB_RD_COMPLETE : STD_LOGIC :='0';
SIGNAL   PORTA_WR_HAPPENED       : STD_LOGIC := '0';
SIGNAL   PORTB_RD_L1             : STD_LOGIC  := '0';
SIGNAL   PORTB_RD_L2             : STD_LOGIC  := '0';
SIGNAL   PORTA_WR_R2             : STD_LOGIC  := '0';
SIGNAL   PORTA_WR_R1             : STD_LOGIC  := '0';

CONSTANT WR_RD_DEEP_COUNT :INTEGER :=8;
CONSTANT WR_DEEP_COUNT    : INTEGER := if_then_else((13 <= 13),WR_RD_DEEP_COUNT,
                                              ((96/96)*WR_RD_DEEP_COUNT));
CONSTANT RD_DEEP_COUNT    : INTEGER := if_then_else((13 <= 13),WR_RD_DEEP_COUNT,
                                              ((96/96)*WR_RD_DEEP_COUNT));

BEGIN

   ADDRA <= WRITE_ADDR(12 DOWNTO 0) ;
   DINA  <= DINA_INT ;
   ADDRB <= READ_ADDR(12 DOWNTO 0) when (DO_READ='1') else (OTHERS=>'0');
   CHECK_DATA <= DO_READ;

  RD_ADDR_GEN_INST:ENTITY work.ADDR_GEN
    GENERIC MAP(
      C_MAX_DEPTH => 4501 ,
      RST_INC => 1    )
    PORT MAP(
       CLK        => CLKB,
       RST        => TB_RST,
       EN         => DO_READ,
       LOAD       => '0',
   	   LOAD_VALUE => ZERO,
       ADDR_OUT   => READ_ADDR
    );

  WR_ADDR_GEN_INST:ENTITY work.ADDR_GEN
    GENERIC MAP( 
      C_MAX_DEPTH => 4501,
      RST_INC => 1    )
    PORT MAP(
       CLK        => CLKA,
       RST        => TB_RST,
  	   EN         => DO_WRITE,
       LOAD       => '0',
       LOAD_VALUE => ZERO,
  	   ADDR_OUT   => WRITE_ADDR
      );

  WR_DATA_GEN_INST:ENTITY work.DATA_GEN 
    GENERIC MAP ( 
       DATA_GEN_WIDTH => 96,
       DOUT_WIDTH     => 96 ,
       DATA_PART_CNT  => 1,
       SEED           => 2)
    PORT MAP (
       CLK      => CLKA,
	   RST      => TB_RST,
       EN       => DO_WRITE,
       DATA_OUT => DINA_INT          
    );


  PORTA_WR_PROCESS: PROCESS(CLKA)
  BEGIN
    IF(RISING_EDGE(CLKA)) THEN
      IF(TB_RST='1') THEN
        PORTA_WR<='1';
      ELSE
        PORTA_WR<=PORTB_RD_COMPLETE;
      END IF;
    END IF;
  END PROCESS;

  PORTB_RD_PROCESS: PROCESS(CLKB)
  BEGIN
    IF(RISING_EDGE(CLKB)) THEN
      IF(TB_RST='1') THEN
        PORTB_RD<='0';
      ELSE
        PORTB_RD<=PORTA_WR_L2;
      END IF;
    END IF;
  END PROCESS;

  PORTB_RD_COMPLETE_LATCH: PROCESS(CLKB) 
    BEGIN
    IF(RISING_EDGE(CLKB)) THEN
      IF(TB_RST='1') THEN
        LATCH_PORTB_RD_COMPLETE<='0';
      ELSIF(PORTB_RD_COMPLETE='1') THEN
        LATCH_PORTB_RD_COMPLETE <='1';
      ELSIF(PORTA_WR_HAPPENED='1') THEN
        LATCH_PORTB_RD_COMPLETE<='0';
      END IF;
    END IF;
  END PROCESS;
  
  PROCESS(CLKA)
  BEGIN
    IF(RISING_EDGE(CLKA)) THEN
      IF(TB_RST='1') THEN
        PORTB_RD_L1 <='0';
        PORTB_RD_L2 <='0';
      ELSE
       PORTB_RD_L1 <= LATCH_PORTB_RD_COMPLETE;
       PORTB_RD_L2 <= PORTB_RD_L1;
      END IF;
   END IF;
  END PROCESS;
  
  PROCESS(CLKB)
  BEGIN
    IF(RISING_EDGE(CLKB)) THEN
      IF(TB_RST='1') THEN
        PORTA_WR_R1 <='0';
        PORTA_WR_R2 <='0';
      ELSE
       PORTA_WR_R1 <= PORTA_WR;
       PORTA_WR_R2 <= PORTA_WR_R1;
      END IF;
   END IF;
  END PROCESS;
  
  PORTA_WR_HAPPENED <= PORTA_WR_R2;
  
  PORTA_WR_COMPLETE_LATCH: PROCESS(CLKA) 
    BEGIN
    IF(RISING_EDGE(CLKA)) THEN
      IF(TB_RST='1') THEN
        LATCH_PORTA_WR_COMPLETE<='0';
      ELSIF(PORTA_WR_COMPLETE='1') THEN
        LATCH_PORTA_WR_COMPLETE <='1';
      --ELSIF(PORTB_RD_HAPPENED='1') THEN
      ELSE
        LATCH_PORTA_WR_COMPLETE<='0';
      END IF;
    END IF;
  END PROCESS;
  
  PROCESS(CLKB)
  BEGIN
    IF(RISING_EDGE(CLKB)) THEN
      IF(TB_RST='1') THEN
        PORTA_WR_L1 <='0';
        PORTA_WR_L2 <='0';
      ELSE
       PORTA_WR_L1 <= LATCH_PORTA_WR_COMPLETE;
       PORTA_WR_L2 <= PORTA_WR_L1;
      END IF;
   END IF;
  END PROCESS;
  
  PROCESS(CLKA)
  BEGIN
    IF(RISING_EDGE(CLKA)) THEN
      IF(TB_RST='1') THEN
        PORTB_RD_R1 <='0';
        PORTB_RD_R2 <='0';
      ELSE
       PORTB_RD_R1 <= PORTB_RD;
       PORTB_RD_R2 <= PORTB_RD_R1;
      END IF;
   END IF;
  END PROCESS;
  
  PORTB_RD_HAPPENED <= PORTB_RD_R2;
  
  PORTB_RD_COMPLETE <= '1' when (count_rd=RD_DEEP_COUNT) else '0';
  
  start_rd_counter: process(clkb)
  begin
    if(rising_edge(clkb)) then
      if(tb_rst='1') then
         incr_rd_cnt <= '0';
       elsif(portb_rd ='1') then
         incr_rd_cnt <='1';
       elsif(portb_rd_complete='1') then
         incr_rd_cnt <='0';
       end if;
    end if;
  end process;
  
  RD_COUNTER: process(clkb)
  begin
    if(rising_edge(clkb)) then
      if(tb_rst='1') then
        count_rd <= 0;
      elsif(incr_rd_cnt='1') then
        count_rd<=count_rd+1;
      end if;
      --if(count_rd=(wr_rd_deep_count)) then
     if(count_rd=(RD_DEEP_COUNT)) then
        count_rd<=0;
      end if;
   end if;
  end process;
  
  DO_READ<='1' when (count_rd <RD_DEEP_COUNT and incr_rd_cnt='1') else '0';
  
  PORTA_WR_COMPLETE <= '1' when (count=WR_DEEP_COUNT) else '0';
  
  start_counter: process(clka)
  begin
    if(rising_edge(clka)) then
      if(tb_rst='1') then
         incr_wr_cnt <= '0';
       elsif(porta_wr ='1') then
         incr_wr_cnt <='1';
       elsif(porta_wr_complete='1') then
         incr_wr_cnt <='0';
       end if;
    end if;
  end process;
  
  COUNTER: process(clka)
  begin
    if(rising_edge(clka)) then
      if(tb_rst='1') then
        count <= 0;
      elsif(incr_wr_cnt='1') then
        count<=count+1;
      end if;
      if(count=(WR_DEEP_COUNT)) then
        count<=0;
      end if;
   end if;
  end process;
  
  DO_WRITE<='1' when (count <WR_DEEP_COUNT and incr_wr_cnt='1') else '0';
  
  
  BEGIN_SHIFT_REG: FOR I IN 0 TO 5 GENERATE
  BEGIN
    DFF_RIGHT: IF I=0 GENERATE
    BEGIN
      SHIFT_INST_0: ENTITY work.REGISTER_LOGIC
        PORT MAP(
           Q    => DO_READ_REG(0),
           CLK  => CLKB,
           RST  => TB_RST,
           D    => DO_READ
        );
     END GENERATE DFF_RIGHT;

    DFF_OTHERS: IF ((I>0) AND (I<=5)) GENERATE
    BEGIN
      SHIFT_INST: ENTITY work.REGISTER_LOGIC
        PORT MAP(
           Q   => DO_READ_REG(I),
           CLK =>CLKB,
           RST =>TB_RST,
           D   =>DO_READ_REG(I-1)
        );
    END GENERATE DFF_OTHERS;
 END GENERATE BEGIN_SHIFT_REG;
  
 REGCE_PROCESS: PROCESS(CLKB) 
   BEGIN
     IF(RISING_EDGE(CLKB)) THEN
       IF(TB_RST='1') THEN
 	     DO_READ_R <= '0';
       ELSE
 	     DO_READ_R <= DO_READ;
       END IF;
     END IF;
 END PROCESS;

 
   ENA <= DO_WRITE ;
 
   WEA(0) <= DO_WRITE ;
 

END ARCHITECTURE;





