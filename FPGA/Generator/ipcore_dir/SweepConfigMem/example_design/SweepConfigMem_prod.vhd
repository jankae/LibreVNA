 
 
 





--------------------------------------------------------------------------------
--
-- BLK MEM GEN v7.1 Core - Top-level wrapper
--
--------------------------------------------------------------------------------
--
-- (c) Copyright 2006-2011 Xilinx, Inc. All rights reserved.
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
--
--------------------------------------------------------------------------------
--
-- Filename: SweepConfigMem_prod.vhd
--
-- Description:
--   This is the top-level BMG wrapper (over BMG core).
--
--------------------------------------------------------------------------------
-- Author: IP Solutions Division
--
-- History: August 31, 2005 - First Release
--------------------------------------------------------------------------------
--
-- Configured Core Parameter Values:
-- (Refer to the SIM Parameters table in the datasheet for more information on
-- the these parameters.)
--    C_FAMILY                    :  spartan6 
--    C_XDEVICEFAMILY             :  spartan6 
--    C_INTERFACE_TYPE            :  0 
--    C_ENABLE_32BIT_ADDRESS      :  0 
--    C_AXI_TYPE                  :  1 
--    C_AXI_SLAVE_TYPE            :  0 
--    C_AXI_ID_WIDTH              :  4 
--    C_MEM_TYPE                  :  1 
--    C_BYTE_SIZE                 :  9 
--    C_ALGORITHM                 :  1 
--    C_PRIM_TYPE                 :  1 
--    C_LOAD_INIT_FILE            :  0 
--    C_INIT_FILE_NAME            :  no_coe_file_loaded 
--    C_USE_DEFAULT_DATA          :  0 
--    C_DEFAULT_DATA              :  0 
--    C_RST_TYPE                  :  SYNC 
--    C_HAS_RSTA                  :  0 
--    C_RST_PRIORITY_A            :  CE 
--    C_RSTRAM_A                  :  0 
--    C_INITA_VAL                 :  0 
--    C_HAS_ENA                   :  1 
--    C_HAS_REGCEA                :  0 
--    C_USE_BYTE_WEA              :  0 
--    C_WEA_WIDTH                 :  1 
--    C_WRITE_MODE_A              :  WRITE_FIRST 
--    C_WRITE_WIDTH_A             :  96 
--    C_READ_WIDTH_A              :  96 
--    C_WRITE_DEPTH_A             :  4501 
--    C_READ_DEPTH_A              :  4501 
--    C_ADDRA_WIDTH               :  13 
--    C_HAS_RSTB                  :  0 
--    C_RST_PRIORITY_B            :  CE 
--    C_RSTRAM_B                  :  0 
--    C_INITB_VAL                 :  0 
--    C_HAS_ENB                   :  0 
--    C_HAS_REGCEB                :  0 
--    C_USE_BYTE_WEB              :  0 
--    C_WEB_WIDTH                 :  1 
--    C_WRITE_MODE_B              :  WRITE_FIRST 
--    C_WRITE_WIDTH_B             :  96 
--    C_READ_WIDTH_B              :  96 
--    C_WRITE_DEPTH_B             :  4501 
--    C_READ_DEPTH_B              :  4501 
--    C_ADDRB_WIDTH               :  13 
--    C_HAS_MEM_OUTPUT_REGS_A     :  0 
--    C_HAS_MEM_OUTPUT_REGS_B     :  0 
--    C_HAS_MUX_OUTPUT_REGS_A     :  0 
--    C_HAS_MUX_OUTPUT_REGS_B     :  0 
--    C_HAS_SOFTECC_INPUT_REGS_A  :  0 
--    C_HAS_SOFTECC_OUTPUT_REGS_B :  0 
--    C_MUX_PIPELINE_STAGES       :  0 
--    C_USE_ECC                   :  0 
--    C_USE_SOFTECC               :  0 
--    C_HAS_INJECTERR             :  0 
--    C_SIM_COLLISION_CHECK       :  ALL 
--    C_COMMON_CLK                :  0 
--    C_DISABLE_WARN_BHV_COLL     :  0 
--    C_DISABLE_WARN_BHV_RANGE    :  0 

--------------------------------------------------------------------------------
-- Library Declarations
--------------------------------------------------------------------------------

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

LIBRARY UNISIM;
USE UNISIM.VCOMPONENTS.ALL;

--------------------------------------------------------------------------------
-- Entity Declaration
--------------------------------------------------------------------------------
ENTITY SweepConfigMem_prod IS
  PORT (
    --Port A
    CLKA       : IN STD_LOGIC;
    RSTA       : IN STD_LOGIC;  --opt port
    ENA        : IN STD_LOGIC;  --optional port
    REGCEA     : IN STD_LOGIC;  --optional port
    WEA        : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    ADDRA      : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
    DINA       : IN STD_LOGIC_VECTOR(95 DOWNTO 0);
    DOUTA      : OUT STD_LOGIC_VECTOR(95 DOWNTO 0);

    --Port B
    CLKB       : IN STD_LOGIC;
    RSTB       : IN STD_LOGIC;  --opt port
    ENB        : IN STD_LOGIC;  --optional port
    REGCEB     : IN STD_LOGIC;  --optional port
    WEB        : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    ADDRB      : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
    DINB       : IN STD_LOGIC_VECTOR(95 DOWNTO 0);
    DOUTB      : OUT STD_LOGIC_VECTOR(95 DOWNTO 0);

    --ECC
    INJECTSBITERR  : IN STD_LOGIC; --optional port
    INJECTDBITERR  : IN STD_LOGIC; --optional port
    SBITERR        : OUT STD_LOGIC; --optional port
    DBITERR        : OUT STD_LOGIC; --optional port
    RDADDRECC      : OUT STD_LOGIC_VECTOR(12 DOWNTO 0); --optional port
 -- AXI BMG Input and Output Port Declarations

    -- AXI Global Signals
    S_ACLK                         : IN  STD_LOGIC;
    S_AXI_AWID                     : IN  STD_LOGIC_VECTOR(3 DOWNTO 0);
    S_AXI_AWADDR                   : IN  STD_LOGIC_VECTOR(31 DOWNTO 0);
    S_AXI_AWLEN                    : IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
    S_AXI_AWSIZE                   : IN  STD_LOGIC_VECTOR(2 DOWNTO 0);
    S_AXI_AWBURST                  : IN  STD_LOGIC_VECTOR(1 DOWNTO 0);
    S_AXI_AWVALID                  : IN  STD_LOGIC;
    S_AXI_AWREADY                  : OUT STD_LOGIC;
    S_AXI_WDATA                    : IN  STD_LOGIC_VECTOR(95  DOWNTO 0);
    S_AXI_WSTRB                    : IN  STD_LOGIC_VECTOR(0  DOWNTO 0);
    S_AXI_WLAST                    : IN  STD_LOGIC;
    S_AXI_WVALID                   : IN  STD_LOGIC;
    S_AXI_WREADY                   : OUT STD_LOGIC;
    S_AXI_BID                      : OUT STD_LOGIC_VECTOR(3  DOWNTO 0):= (OTHERS => '0');
    S_AXI_BRESP                    : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    S_AXI_BVALID                   : OUT STD_LOGIC;
    S_AXI_BREADY                   : IN  STD_LOGIC;

    -- AXI Full/Lite Slave Read (Write side)
    S_AXI_ARID                     : IN  STD_LOGIC_VECTOR(3  DOWNTO 0);
    S_AXI_ARADDR                   : IN  STD_LOGIC_VECTOR(31 DOWNTO 0);
    S_AXI_ARLEN                    : IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
    S_AXI_ARSIZE                   : IN  STD_LOGIC_VECTOR(2 DOWNTO 0);
    S_AXI_ARBURST                  : IN  STD_LOGIC_VECTOR(1 DOWNTO 0);
    S_AXI_ARVALID                  : IN  STD_LOGIC;
    S_AXI_ARREADY                  : OUT STD_LOGIC;
    S_AXI_RID                      : OUT STD_LOGIC_VECTOR(3  DOWNTO 0):= (OTHERS => '0');
    S_AXI_RDATA                    : OUT STD_LOGIC_VECTOR(95  DOWNTO 0);
    S_AXI_RRESP                    : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    S_AXI_RLAST                    : OUT STD_LOGIC;
    S_AXI_RVALID                   : OUT STD_LOGIC;
    S_AXI_RREADY                   : IN  STD_LOGIC;

    -- AXI Full/Lite Sideband Signals
    S_AXI_INJECTSBITERR            : IN  STD_LOGIC;
    S_AXI_INJECTDBITERR            : IN  STD_LOGIC;
    S_AXI_SBITERR                  : OUT STD_LOGIC;
    S_AXI_DBITERR                  : OUT STD_LOGIC;
    S_AXI_RDADDRECC                : OUT STD_LOGIC_VECTOR(12  DOWNTO 0);
    S_ARESETN                      : IN  STD_LOGIC


  );

END SweepConfigMem_prod;


ARCHITECTURE xilinx OF SweepConfigMem_prod IS

  COMPONENT SweepConfigMem_exdes IS
  PORT (
      --Port A
    ENA            : IN STD_LOGIC;  --opt port
  
    WEA            : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    ADDRA          : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
  
    DINA           : IN STD_LOGIC_VECTOR(95 DOWNTO 0);

  
    CLKA       : IN STD_LOGIC;

  
      --Port B
    ADDRB          : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
    DOUTB          : OUT STD_LOGIC_VECTOR(95 DOWNTO 0);
    CLKB           : IN STD_LOGIC



  );
  END COMPONENT;

BEGIN

  bmg0 : SweepConfigMem_exdes
    PORT MAP (
      --Port A
      ENA        => ENA,
  
      WEA        => WEA,
      ADDRA      => ADDRA,
  
      DINA       => DINA,

      CLKA       => CLKA,
  
      --Port B
      ADDRB      => ADDRB,
      DOUTB      => DOUTB,
      CLKB       => CLKB



      );
END xilinx;
