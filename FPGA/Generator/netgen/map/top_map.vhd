--------------------------------------------------------------------------------
-- Copyright (c) 1995-2013 Xilinx, Inc.  All rights reserved.
--------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /    Vendor: Xilinx
-- \   \   \/     Version: P.20131013
--  \   \         Application: netgen
--  /   /         Filename: top_map.vhd
-- /___/   /\     Timestamp: Fri Jun 10 23:07:08 2022
-- \   \  /  \ 
--  \___\/\___\
--             
-- Command	: -filter /home/jan/Projekte/LibreVNA/FPGA/Generator/iseconfig/filter.filter -intstyle ise -s 2 -pcf top.pcf -rpw 100 -tpw 0 -ar Structure -tm top -w -dir netgen/map -ofmt vhdl -sim top_map.ncd top_map.vhd 
-- Device	: 6slx9tqg144-2 (PRODUCTION 1.23 2013-10-13)
-- Input file	: top_map.ncd
-- Output file	: /home/jan/Projekte/LibreVNA/FPGA/Generator/netgen/map/top_map.vhd
-- # of Entities	: 1
-- Design Name	: top
-- Xilinx	: /opt/Xilinx/14.7/ISE_DS/ISE/
--             
-- Purpose:    
--     This VHDL netlist is a verification model and uses simulation 
--     primitives which may not represent the true implementation of the 
--     device, however the netlist is functionally correct and should not 
--     be modified. This file cannot be synthesized and should only be used 
--     with supported simulation tools.
--             
-- Reference:  
--     Command Line Tools User Guide, Chapter 23
--     Synthesis and Simulation Design Guide, Chapter 6
--             
--------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library SIMPRIM;
use SIMPRIM.VCOMPONENTS.ALL;
use SIMPRIM.VPACKAGE.ALL;

entity top is
  port (
    CLK : in STD_LOGIC := 'X'; 
    RESET : in STD_LOGIC := 'X'; 
    MCU_MOSI : in STD_LOGIC := 'X'; 
    MCU_NSS : in STD_LOGIC := 'X'; 
    MCU_SCK : in STD_LOGIC := 'X'; 
    MCU_AUX1 : in STD_LOGIC := 'X'; 
    MCU_AUX2 : in STD_LOGIC := 'X'; 
    MCU_AUX3 : in STD_LOGIC := 'X'; 
    PORT2_SDO : in STD_LOGIC := 'X'; 
    PORT1_SDO : in STD_LOGIC := 'X'; 
    LO1_MUX : in STD_LOGIC := 'X'; 
    LO1_LD : in STD_LOGIC := 'X'; 
    SOURCE_LD : in STD_LOGIC := 'X'; 
    SOURCE_MUX : in STD_LOGIC := 'X'; 
    REF_SDO : in STD_LOGIC := 'X'; 
    MCU_INTR : out STD_LOGIC; 
    MCU_MISO : out STD_LOGIC; 
    PORT2_CONVSTART : out STD_LOGIC; 
    PORT2_SCLK : out STD_LOGIC; 
    PORT2_MIX2_EN : out STD_LOGIC; 
    PORT2_MIX1_EN : out STD_LOGIC; 
    PORT1_CONVSTART : out STD_LOGIC; 
    PORT1_SCLK : out STD_LOGIC; 
    PORT1_MIX2_EN : out STD_LOGIC; 
    PORT1_MIX1_EN : out STD_LOGIC; 
    LO1_RF_EN : out STD_LOGIC; 
    LO1_CLK : out STD_LOGIC; 
    LO1_MOSI : out STD_LOGIC; 
    LO1_LE : out STD_LOGIC; 
    LO1_CE : out STD_LOGIC; 
    REF_MIX2_EN : out STD_LOGIC; 
    REF_MIX1_EN : out STD_LOGIC; 
    AMP_PWDN : out STD_LOGIC; 
    PORT1_SELECT : out STD_LOGIC; 
    PORT2_SELECT : out STD_LOGIC; 
    PORT_SELECT1 : out STD_LOGIC; 
    PORT_SELECT2 : out STD_LOGIC; 
    BAND_SELECT_HIGH : out STD_LOGIC; 
    BAND_SELECT_LOW : out STD_LOGIC; 
    FILT_OUT_C1 : out STD_LOGIC; 
    FILT_OUT_C2 : out STD_LOGIC; 
    FILT_IN_C1 : out STD_LOGIC; 
    FILT_IN_C2 : out STD_LOGIC; 
    SOURCE_RF_EN : out STD_LOGIC; 
    SOURCE_CLK : out STD_LOGIC; 
    SOURCE_MOSI : out STD_LOGIC; 
    SOURCE_LE : out STD_LOGIC; 
    SOURCE_CE : out STD_LOGIC; 
    REF_CONVSTART : out STD_LOGIC; 
    REF_SCLK : out STD_LOGIC; 
    LEDS : out STD_LOGIC_VECTOR ( 7 downto 0 ); 
    ATTENUATION : out STD_LOGIC_VECTOR ( 6 downto 0 ) 
  );
end top;

architecture Structure of top is
  signal clk_pll : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_8_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_9_0 : STD_LOGIC; 
  signal mod_reset : STD_LOGIC; 
  signal SPI_SPI_n0066_inv : STD_LOGIC; 
  signal MCU_SCK_IBUF_BUFG_5814 : STD_LOGIC; 
  signal SPI_SPI_GND_18_o_bit_cnt_3_equal_4_o : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_8_0 : STD_LOGIC; 
  signal N87 : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_11_0 : STD_LOGIC; 
  signal SPI_n0539_inv11_2_cepot_5823 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_4_dpot_5825 : STD_LOGIC; 
  signal SPI_n0539_inv11_2_rstpot_5827 : STD_LOGIC; 
  signal PORT_SELECT1_OBUF_5828 : STD_LOGIC; 
  signal SPI_SOURCE_PORTSELECT_5829 : STD_LOGIC; 
  signal SPI_AMP_SHDN_5830 : STD_LOGIC; 
  signal PORT_SELECT2_OBUF_0 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_9_dpot_5833 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_8_dpot_5834 : STD_LOGIC; 
  signal LEDS_5_OBUF_5836 : STD_LOGIC; 
  signal LEDS_4_OBUF_0 : STD_LOGIC; 
  signal SOURCE_LE_OBUF_5838 : STD_LOGIC; 
  signal Source_sclk_5839 : STD_LOGIC; 
  signal MCU_SCK_IBUF_0 : STD_LOGIC; 
  signal Source_latch_5842 : STD_LOGIC; 
  signal MCU_NSS_IBUF_0 : STD_LOGIC; 
  signal SOURCE_CLK_OBUF_0 : STD_LOGIC; 
  signal Source_n0185_inv : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_14_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_15_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_22_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_23_0 : STD_LOGIC; 
  signal Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o : STD_LOGIC; 
  signal inv_clk_locked : STD_LOGIC; 
  signal SPI_n0394 : STD_LOGIC; 
  signal SPI_last_NSS_5865 : STD_LOGIC; 
  signal Sync_NSS_sync_line_2_0 : STD_LOGIC; 
  signal Inst_ResetDelay_OUT_RESET_5868 : STD_LOGIC; 
  signal SPI_n0588_inv1_cepot_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_13_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_12_0 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_10_dpot_5875 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_1_dpot_5876 : STD_LOGIC; 
  signal Source_n0181_inv : STD_LOGIC; 
  signal Source_n0127 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_11_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_4_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_24_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_25_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_18_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_2_0 : STD_LOGIC; 
  signal SPI_SPI_data_valid_0_5893 : STD_LOGIC; 
  signal SPI_SPI_data_synced_2_5894 : STD_LOGIC; 
  signal SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o : STD_LOGIC; 
  signal SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0 : STD_LOGIC; 
  signal SPI_n0652_inv : STD_LOGIC; 
  signal SPI_Mcount_selected_register1 : STD_LOGIC; 
  signal SPI_state_FSM_FFd1_5903 : STD_LOGIC; 
  signal SPI_Mcount_selected_register_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_5_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_3_0 : STD_LOGIC; 
  signal SPI_n0474_inv_5916 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_3_dpot_5919 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_2_dpot_5920 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_7_dpot_5923 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_6_dpot_5924 : STD_LOGIC; 
  signal LO1_MOSI_OBUF_5926 : STD_LOGIC; 
  signal SPI_MOD_ENABLE_5927 : STD_LOGIC; 
  signal MCU_MOSI_IBUF_0 : STD_LOGIC; 
  signal Inst_ResetDelay_OUT_RESET_rstpot_5930 : STD_LOGIC; 
  signal LEDS_1_OBUF_5931 : STD_LOGIC; 
  signal RESET_IBUF_0 : STD_LOGIC; 
  signal fpga_select : STD_LOGIC; 
  signal aux1_sync_aux2_sync_OR_8_o_inv_0 : STD_LOGIC; 
  signal SPI_SPI_Mcount_bit_cnt1 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_6_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_7_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_19_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_21_0 : STD_LOGIC; 
  signal SPI_SPI_Mcount_bit_cnt2 : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_6_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_16_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_17_0 : STD_LOGIC; 
  signal SPI_SPI_Mcount_bit_cnt3 : STD_LOGIC; 
  signal SPI_n0539_inv11_cepot_5955 : STD_LOGIC; 
  signal SPI_n0487_inv_cepot_0 : STD_LOGIC; 
  signal LO1_LE_OBUF_5957 : STD_LOGIC; 
  signal LO1_CLK_OBUF_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_0_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_10_0 : STD_LOGIC; 
  signal SPI_n0539_inv11_1_cepot_0 : STD_LOGIC; 
  signal source_reload_5964 : STD_LOGIC; 
  signal Source_done_int_5965 : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot6_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_1_0 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_20_0 : STD_LOGIC; 
  signal SPI_mod_table_write_5972 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_INDEX_7_0 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_INDEX_6_0 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_INDEX_5_0 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_INDEX_4_0 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_134_0 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_135_0 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_133_0 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_132_0 : STD_LOGIC; 
  signal Modulation_OUTPUT_SAMPLE_7_0 : STD_LOGIC; 
  signal Modulation_OUTPUT_SAMPLE_6_0 : STD_LOGIC; 
  signal Modulation_OUTPUT_SAMPLE_5_0 : STD_LOGIC; 
  signal Modulation_OUTPUT_SAMPLE_4_0 : STD_LOGIC; 
  signal mod_sample_latch : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_0_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_1_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_2_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_3_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_4_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_5_0 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_Q_6314 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_6_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_7_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_8_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_9_0 : STD_LOGIC; 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_0 : STD_LOGIC; 
  signal Modulation_THRESHOLD_CROSSED_6320 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Q_6321 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Q_6322 : STD_LOGIC; 
  signal Modulation_Result_0_1_0 : STD_LOGIC; 
  signal Modulation_Result_1_1_0 : STD_LOGIC; 
  signal Modulation_Result_2_1_0 : STD_LOGIC; 
  signal Modulation_Mcount_read_pos_cy_3_Q_6326 : STD_LOGIC; 
  signal Modulation_Result_3_1_0 : STD_LOGIC; 
  signal Modulation_Result_4_1_0 : STD_LOGIC; 
  signal Modulation_Result_5_1_0 : STD_LOGIC; 
  signal Modulation_Result_6_1_0 : STD_LOGIC; 
  signal Modulation_Mcount_read_pos_cy_7_Q_6331 : STD_LOGIC; 
  signal Modulation_Result_7_1_0 : STD_LOGIC; 
  signal Modulation_Result_8_1_0 : STD_LOGIC; 
  signal Modulation_Result_9_1_0 : STD_LOGIC; 
  signal Modulation_Result_10_1_0 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Q_6339 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Q_6344 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Q_6349 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Q_6354 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Q_6356 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Q_6357 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_0 : STD_LOGIC; 
  signal Modulation_Mcount_write_pos_cy_3_Q_6359 : STD_LOGIC; 
  signal Modulation_Mcount_write_pos_cy_7_Q_6360 : STD_LOGIC; 
  signal MainCLK_clkin1_0 : STD_LOGIC; 
  signal PORT1_SDO_IBUF_0 : STD_LOGIC; 
  signal PORT2_SDO_IBUF_0 : STD_LOGIC; 
  signal SPI_SOURCE_CE_EN_6366 : STD_LOGIC; 
  signal LEDS_2_OBUF_0 : STD_LOGIC; 
  signal SOURCE_MUX_IBUF_0 : STD_LOGIC; 
  signal SPI_SOURCE_RF_EN_6369 : STD_LOGIC; 
  signal LEDS_3_OBUF_0 : STD_LOGIC; 
  signal BAND_SELECT_HIGH_OBUF_6371 : STD_LOGIC; 
  signal FILT_IN_C2_OBUF_6372 : STD_LOGIC; 
  signal SOURCE_MOSI_OBUF_6373 : STD_LOGIC; 
  signal SPI_SOURCE_BANDSELECT_6374 : STD_LOGIC; 
  signal ATTENUATION_0_OBUF_6375 : STD_LOGIC; 
  signal ATTENUATION_1_OBUF_6376 : STD_LOGIC; 
  signal ATTENUATION_2_OBUF_6377 : STD_LOGIC; 
  signal ATTENUATION_3_OBUF_6378 : STD_LOGIC; 
  signal ATTENUATION_4_OBUF_6379 : STD_LOGIC; 
  signal ATTENUATION_5_OBUF_6380 : STD_LOGIC; 
  signal ATTENUATION_6_OBUF_6381 : STD_LOGIC; 
  signal MCU_AUX1_IBUF_0 : STD_LOGIC; 
  signal MCU_AUX2_IBUF_0 : STD_LOGIC; 
  signal MCU_AUX3_IBUF_0 : STD_LOGIC; 
  signal LO1_MUX_IBUF_0 : STD_LOGIC; 
  signal SPI_INTERRUPT_ASSERTED_6389 : STD_LOGIC; 
  signal MCU_MISO_OBUFT_0 : STD_LOGIC; 
  signal REF_SDO_IBUF_0 : STD_LOGIC; 
  signal MainCLK_clk0 : STD_LOGIC; 
  signal MainCLK_clkfb : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_ML_NEW_DIVCLK : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_ML_NEW_O : STD_LOGIC; 
  signal MainCLK_clkfx : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o116_0 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o61 : STD_LOGIC; 
  signal SPI_state_FSM_FFd2_6400 : STD_LOGIC; 
  signal SPI_NSS_last_NSS_AND_33_o : STD_LOGIC; 
  signal SPI_SPI_COMPLETE_6402 : STD_LOGIC; 
  signal SPI_n0440_inv1_cepot : STD_LOGIC; 
  signal SPI_mod_first_byte_6405 : STD_LOGIC; 
  signal SPI_n0539_inv11_1_rstpot_6425 : STD_LOGIC; 
  signal SPI_mod_second_byte_6429 : STD_LOGIC; 
  signal SPI_spi_buf_in_15_Q : STD_LOGIC; 
  signal SPI_n0487_inv_rstpot_6436 : STD_LOGIC; 
  signal SPI_n0699_inv : STD_LOGIC; 
  signal SPI_spi_buf_in_1_Q : STD_LOGIC; 
  signal SPI_n0419 : STD_LOGIC; 
  signal SPI_spi_buf_in_2_Q : STD_LOGIC; 
  signal SPI_spi_buf_in_3_Q : STD_LOGIC; 
  signal SPI_spi_buf_in_4_Q : STD_LOGIC; 
  signal Source_n0148_inv11_cepot : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot_6449 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_10_dpot_6452 : STD_LOGIC; 
  signal SPI_n0539_inv11_rstpot_6453 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_11_dpot_6454 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_12_dpot_6455 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_13_dpot_6456 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_14_dpot_6457 : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot1 : STD_LOGIC; 
  signal Source_n0148_inv11_cepot1 : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot2 : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot3 : STD_LOGIC; 
  signal Source_n0148_inv11_cepot2 : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot4 : STD_LOGIC; 
  signal Source_n0148_inv11_cepot3 : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot5 : STD_LOGIC; 
  signal N85 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o115_6492 : STD_LOGIC; 
  signal N61 : STD_LOGIC; 
  signal N79 : STD_LOGIC; 
  signal N77 : STD_LOGIC; 
  signal SPI_n0419_13_113 : STD_LOGIC; 
  signal SPI_GND_17_o_word_cnt_4_equal_23_o : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_94_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_95_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_88_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_89_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_90_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_91_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_92_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_100_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_93_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_101_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_102_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_103_0 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_1_dpot_6534 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_2_dpot_6535 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_3_dpot_6536 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_4_dpot_6537 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_5_dpot_6538 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_6_dpot_6539 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_7_dpot_6540 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_8_dpot_6541 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_9_dpot_6542 : STD_LOGIC; 
  signal SPI_n0419_13_1 : STD_LOGIC; 
  signal SPI_n0419_13_111 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_16_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_24_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_17_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_25_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_18_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_26_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_19_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_27_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_20_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_28_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_21_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_29_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_22_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_30_0 : STD_LOGIC; 
  signal SPI_n0419_13_112 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_54_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_23_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_31_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_55_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_40_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_64_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_41_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_65_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_42_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_66_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_43_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_67_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_52_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_60_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_53_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_61_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_62_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_63_0 : STD_LOGIC; 
  signal SPI_SPI_data_synced_0_6630 : STD_LOGIC; 
  signal Sync_AUX1_sync_line_2_1_6638 : STD_LOGIC; 
  signal Sync_AUX1_sync_line_2_2_6639 : STD_LOGIC; 
  signal SPI_SPI_data_valid_1_inv : STD_LOGIC; 
  signal SPI_SPI_data_valid_1_6641 : STD_LOGIC; 
  signal SPI_SPI_n0059_inv : STD_LOGIC; 
  signal SPI_SPI_data_10_0 : STD_LOGIC; 
  signal SPI_SPI_data_11_0 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_Q_6649 : STD_LOGIC; 
  signal N24 : STD_LOGIC; 
  signal N26 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_0_dpot_6652 : STD_LOGIC; 
  signal SPI_n0484_inv_6653 : STD_LOGIC; 
  signal SPI_n0621 : STD_LOGIC; 
  signal SPI_n0578_inv : STD_LOGIC; 
  signal SPI_n0578_inv1_6656 : STD_LOGIC; 
  signal SPI_n0578_inv11 : STD_LOGIC; 
  signal SPI_n0578_inv12 : STD_LOGIC; 
  signal Inst_ResetDelay_OUT_RESET_1_6659 : STD_LOGIC; 
  signal Inst_ResetDelay_OUT_RESET_2_6660 : STD_LOGIC; 
  signal source_reloaded_source_reloaded_last_AND_35_o : STD_LOGIC; 
  signal source_reloaded_last_6662 : STD_LOGIC; 
  signal Sync_NSS_sync_line_2_1_6663 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_11_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_120_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_121_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_122_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_123_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_124_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_125_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_126_0 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_127_0 : STD_LOGIC; 
  signal Modulation_OVERFLOW_6686 : STD_LOGIC; 
  signal Modulation_UNDERFLOW_6687 : STD_LOGIC; 
  signal Modulation_n00682 : STD_LOGIC; 
  signal N22 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_0_dpot_6690 : STD_LOGIC; 
  signal SPI_n0419_13_11_6691 : STD_LOGIC; 
  signal Sync_AUX2_sync_line_2_1_6692 : STD_LOGIC; 
  signal SPI_SPI_COMPLETE_1_6693 : STD_LOGIC; 
  signal SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_6694 : STD_LOGIC; 
  signal SPI_state_FSM_FFd2_1_6695 : STD_LOGIC; 
  signal SPI_state_FSM_FFd1_1_6696 : STD_LOGIC; 
  signal Source_n01272 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o11 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_Q_6699 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o112_6700 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o113_6701 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o114_6702 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o117_6703 : STD_LOGIC; 
  signal N12 : STD_LOGIC; 
  signal SPI_SPI_data_8_0 : STD_LOGIC; 
  signal SPI_SPI_data_9_0 : STD_LOGIC; 
  signal N8 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_Q_6730 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_Q_6731 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_Q_6732 : STD_LOGIC; 
  signal Source_done_int_1_6733 : STD_LOGIC; 
  signal SPI_n0565_inv13 : STD_LOGIC; 
  signal SPI_SPI_data_synced_0_inv : STD_LOGIC; 
  signal SPI_state_FSM_FFd1_In_6739 : STD_LOGIC; 
  signal SPI_state_FSM_FFd2_In : STD_LOGIC; 
  signal SPI_selected_register_0_1_6741 : STD_LOGIC; 
  signal SPI_selected_register_1_1_6742 : STD_LOGIC; 
  signal Source_n0170_inv1_cepot_6743 : STD_LOGIC; 
  signal N10 : STD_LOGIC; 
  signal SPI_n0453_inv : STD_LOGIC; 
  signal Source_n0204_inv : STD_LOGIC; 
  signal Source_done_int_glue_rst_6792 : STD_LOGIC; 
  signal Source_n0134 : STD_LOGIC; 
  signal Source_n0170_inv1_rstpot_6820 : STD_LOGIC; 
  signal N20 : STD_LOGIC; 
  signal Modulation_NEW_OUTPUT_6822 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_15_dpot_6828 : STD_LOGIC; 
  signal SPI_n0539_inv111 : STD_LOGIC; 
  signal Sync_AUX2_sync_line_2_2_6833 : STD_LOGIC; 
  signal N14 : STD_LOGIC; 
  signal Mmux_Z_3_o_SOURCE_MUX_MUX_194_o1 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_5_dpot_6837 : STD_LOGIC; 
  signal N6 : STD_LOGIC; 
  signal N16 : STD_LOGIC; 
  signal N93 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o111_6847 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_Q_6848 : STD_LOGIC; 
  signal N83 : STD_LOGIC; 
  signal N4 : STD_LOGIC; 
  signal N0 : STD_LOGIC; 
  signal N91 : STD_LOGIC; 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO8 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO9 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO10 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO11 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO12 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO13 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO14 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO15 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOPBDOP1 : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT : STD_LOGIC;
 
  signal ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT : STD_LOGIC;
 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi_576 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_0_Q_575 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi1_570 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_1_Q_569 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi2_564 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_2_Q_563 : STD_LOGIC; 
  signal ProtoComp29_CYINITVCC_1 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi3_556 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_3_Q_555 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi4_595 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_4_Q_594 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi5_591 : STD_LOGIC; 
  signal Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_5_Q_590 : STD_LOGIC; 
  signal Modulation_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_0_Q_621 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_1_1_617 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_2_Q_613 : STD_LOGIC; 
  signal ProtoComp31_CYINITVCC_1 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_1_603 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_1_643 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_1_639 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_1_635 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_7_1_625 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_8_1_658 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_1_654 : STD_LOGIC; 
  signal Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_1_650 : STD_LOGIC; 
  signal Modulation_read_pos_1_rt_673 : STD_LOGIC; 
  signal Modulation_read_pos_2_rt_670 : STD_LOGIC; 
  signal Modulation_Result_0_1 : STD_LOGIC; 
  signal Modulation_Result_1_1 : STD_LOGIC; 
  signal Modulation_Result_2_1 : STD_LOGIC; 
  signal Modulation_Result_3_1 : STD_LOGIC; 
  signal ProtoComp34_CYINITGND_0 : STD_LOGIC; 
  signal Modulation_read_pos_3_rt_661 : STD_LOGIC; 
  signal Modulation_read_pos_4_rt_698 : STD_LOGIC; 
  signal Modulation_read_pos_5_rt_695 : STD_LOGIC; 
  signal Modulation_read_pos_6_rt_692 : STD_LOGIC; 
  signal Modulation_Result_4_1 : STD_LOGIC; 
  signal Modulation_Result_5_1 : STD_LOGIC; 
  signal Modulation_Result_6_1 : STD_LOGIC; 
  signal Modulation_Result_7_1 : STD_LOGIC; 
  signal Modulation_read_pos_7_rt_683 : STD_LOGIC; 
  signal Modulation_read_pos_8_rt_714 : STD_LOGIC; 
  signal Modulation_read_pos_9_rt_711 : STD_LOGIC; 
  signal Modulation_read_pos_10_rt_709 : STD_LOGIC; 
  signal Modulation_Result_8_1 : STD_LOGIC; 
  signal Modulation_Result_9_1 : STD_LOGIC; 
  signal Modulation_Result_10_1 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_0_Q_738 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_1_Q_734 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_2_Q_730 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_0_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_1_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_2_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_3_Q : STD_LOGIC; 
  signal ProtoComp37_CYINITGND_0 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_3_Q_720 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_4_Q_760 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_5_Q_756 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_6_Q_752 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_4_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_5_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_6_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_7_Q : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_7_Q_742 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_8_Q_782 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_9_Q_778 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_10_Q_774 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_8_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_9_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_10_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_11_Q : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_11_Q_764 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_12_Q_804 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_13_Q_800 : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_14_Q_796 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_12_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_13_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_14_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_15_Q : STD_LOGIC; 
  signal Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_15_Q_786 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_16_rt_822 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_17_rt_819 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_18_rt_816 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_16_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_17_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_18_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_19_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_19_rt_807 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_20_rt_844 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_21_rt_841 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_22_rt_838 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_20_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_21_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_22_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_23_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_23_rt_829 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_24_rt_860 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_25_rt_857 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_rt_855 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_24_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_25_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_Q : STD_LOGIC; 
  signal Modulation_write_pos_1_rt_883 : STD_LOGIC; 
  signal Modulation_write_pos_2_rt_879 : STD_LOGIC; 
  signal ProtoComp39_CYINITGND_0 : STD_LOGIC; 
  signal Modulation_write_pos_3_rt_867 : STD_LOGIC; 
  signal Modulation_write_pos_4_rt_916 : STD_LOGIC; 
  signal Modulation_write_pos_5_rt_912 : STD_LOGIC; 
  signal Modulation_write_pos_6_rt_908 : STD_LOGIC; 
  signal Modulation_write_pos_7_rt_896 : STD_LOGIC; 
  signal Modulation_write_pos_8_rt_938 : STD_LOGIC; 
  signal Modulation_write_pos_9_rt_934 : STD_LOGIC; 
  signal Modulation_write_pos_10_rt_931 : STD_LOGIC; 
  signal MainCLK_clkin1 : STD_LOGIC; 
  signal PORT1_SDO_IBUF_957 : STD_LOGIC; 
  signal PORT2_SDO_IBUF_960 : STD_LOGIC; 
  signal LEDS_2_OBUF_975 : STD_LOGIC; 
  signal RESET_IBUF_980 : STD_LOGIC; 
  signal SOURCE_MUX_IBUF_985 : STD_LOGIC; 
  signal LEDS_3_OBUF_994 : STD_LOGIC; 
  signal MCU_AUX1_IBUF_1055 : STD_LOGIC; 
  signal MCU_AUX2_IBUF_1058 : STD_LOGIC; 
  signal MCU_AUX3_IBUF_1061 : STD_LOGIC; 
  signal LO1_MUX_IBUF_1064 : STD_LOGIC; 
  signal MCU_MOSI_IBUF_1072 : STD_LOGIC; 
  signal MCU_SCK_IBUF_1075 : STD_LOGIC; 
  signal MCU_NSS_IBUF_1078 : STD_LOGIC; 
  signal REF_SDO_IBUF_1083 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS0 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS1 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS2 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS3 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS4 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS5 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS6 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_STATUS7 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLK90 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_PSDONE : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLKDV : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLKFX180 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLK270 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLK180 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLK2X : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLK2X180 : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_PSEN_INT : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLKFB_INT : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_CLKIN_INT : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_PSINCDEC_INT : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_PSCLK_INT : STD_LOGIC; 
  signal MainCLK_dcm_sp_inst_DSSEN : STD_LOGIC; 
  signal Source_latched_regs_17_dpot_1131 : STD_LOGIC; 
  signal Source_latched_regs_18_dpot_1124 : STD_LOGIC; 
  signal Source_latched_regs_19_dpot_1109 : STD_LOGIC; 
  signal Source_latched_regs_82_dpot_1158 : STD_LOGIC; 
  signal Source_latched_regs_79_dpot_1151 : STD_LOGIC; 
  signal Source_latched_regs_80_dpot_1144 : STD_LOGIC; 
  signal Source_latched_regs_81_dpot_1137 : STD_LOGIC; 
  signal Source_latched_regs_76_dpot_1186 : STD_LOGIC; 
  signal Source_latched_regs_77_dpot_1179 : STD_LOGIC; 
  signal Source_latched_regs_78_dpot_1170 : STD_LOGIC; 
  signal PORT_SELECT2_OBUF_1199 : STD_LOGIC; 
  signal Source_latched_regs_23_dpot_1226 : STD_LOGIC; 
  signal Source_latched_regs_20_dpot_1219 : STD_LOGIC; 
  signal Source_latched_regs_21_dpot_1212 : STD_LOGIC; 
  signal Source_latched_regs_22_dpot_1205 : STD_LOGIC; 
  signal Source_latched_regs_27_dpot_1256 : STD_LOGIC; 
  signal Source_latched_regs_24_dpot_1249 : STD_LOGIC; 
  signal Source_latched_regs_25_dpot_1242 : STD_LOGIC; 
  signal Source_latched_regs_26_dpot_1235 : STD_LOGIC; 
  signal Source_latched_regs_16_dpot_1286 : STD_LOGIC; 
  signal Source_latched_regs_13_dpot_1279 : STD_LOGIC; 
  signal Source_latched_regs_14_dpot_1272 : STD_LOGIC; 
  signal Source_latched_regs_15_dpot_1265 : STD_LOGIC; 
  signal Source_latched_regs_0_pack_4 : STD_LOGIC; 
  signal Source_latched_regs_0_dpot_1306 : STD_LOGIC; 
  signal SPI_SOURCE_BANDSELECT_dpot_1318 : STD_LOGIC; 
  signal Source_latched_regs_95_dpot_1331 : STD_LOGIC; 
  signal Source_latched_regs_99_dpot_1364 : STD_LOGIC; 
  signal Source_latched_regs_96_dpot_1357 : STD_LOGIC; 
  signal Source_latched_regs_97_dpot_1350 : STD_LOGIC; 
  signal Source_latched_regs_98_dpot_1343 : STD_LOGIC; 
  signal Source_latched_regs_90_dpot_1394 : STD_LOGIC; 
  signal Source_latched_regs_87_dpot_1387 : STD_LOGIC; 
  signal Source_latched_regs_88_dpot_1380 : STD_LOGIC; 
  signal Source_latched_regs_89_dpot_1373 : STD_LOGIC; 
  signal Source_latched_regs_119_dpot_1434 : STD_LOGIC; 
  signal Source_latched_regs_116_dpot_1427 : STD_LOGIC; 
  signal Source_latched_regs_117_dpot_1420 : STD_LOGIC; 
  signal Source_latched_regs_118_dpot_1413 : STD_LOGIC; 
  signal SPI_SOURCE_FILTER_1_dpot_1447 : STD_LOGIC; 
  signal Source_latched_regs_31_dpot_1471 : STD_LOGIC; 
  signal Source_latched_regs_28_dpot_1464 : STD_LOGIC; 
  signal Source_latched_regs_29_dpot_1457 : STD_LOGIC; 
  signal Source_latched_regs_30_dpot_1450 : STD_LOGIC; 
  signal Source_latched_regs_12_dpot_1501 : STD_LOGIC; 
  signal Source_latched_regs_9_dpot_1494 : STD_LOGIC; 
  signal Source_latched_regs_10_dpot_1487 : STD_LOGIC; 
  signal Source_latched_regs_11_dpot_1480 : STD_LOGIC; 
  signal Source_latched_regs_94_dpot_1544 : STD_LOGIC; 
  signal Source_latched_regs_91_dpot_1537 : STD_LOGIC; 
  signal Source_latched_regs_92_dpot_1530 : STD_LOGIC; 
  signal Source_latched_regs_93_dpot_1523 : STD_LOGIC; 
  signal Source_latched_regs_86_dpot_1574 : STD_LOGIC; 
  signal Source_latched_regs_83_dpot_1567 : STD_LOGIC; 
  signal Source_latched_regs_84_dpot_1560 : STD_LOGIC; 
  signal Source_latched_regs_85_dpot_1553 : STD_LOGIC; 
  signal Source_latched_regs_123_dpot_1634 : STD_LOGIC; 
  signal Source_latched_regs_120_dpot_1627 : STD_LOGIC; 
  signal Source_latched_regs_121_dpot_1620 : STD_LOGIC; 
  signal Source_latched_regs_122_dpot_1613 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_12_dpot1_1662 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_13_dpot1_1657 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_14_dpot1_1650 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_11_dpot1_1645 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_0_dpot_1694 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_1_dpot_1687 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_2_dpot_1680 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_3_dpot_1671 : STD_LOGIC; 
  signal Source_latched_regs_8_dpot_1721 : STD_LOGIC; 
  signal Source_latched_regs_5_dpot_1714 : STD_LOGIC; 
  signal Source_latched_regs_6_dpot_1707 : STD_LOGIC; 
  signal Source_latched_regs_7_dpot_1700 : STD_LOGIC; 
  signal Source_sclk_rstpot_1751 : STD_LOGIC; 
  signal Source_done_int_glue_rst_pack_13 : STD_LOGIC; 
  signal Source_latch_rstpot_1738 : STD_LOGIC; 
  signal Source_n0148_inv11_rstpot6 : STD_LOGIC; 
  signal Source_latched_regs_103_dpot_1786 : STD_LOGIC; 
  signal Source_latched_regs_100_dpot_1779 : STD_LOGIC; 
  signal Source_latched_regs_101_dpot_1772 : STD_LOGIC; 
  signal Source_latched_regs_102_dpot_1765 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_105_dpot_1822 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_106_dpot_1815 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_107_dpot_1806 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_104_dpot_1799 : STD_LOGIC; 
  signal Source_latched_regs_107_dpot_1846 : STD_LOGIC; 
  signal Source_latched_regs_104_dpot_1839 : STD_LOGIC; 
  signal Source_latched_regs_105_dpot_1832 : STD_LOGIC; 
  signal Source_latched_regs_106_dpot_1825 : STD_LOGIC; 
  signal Source_latched_regs_111_dpot_1876 : STD_LOGIC; 
  signal Source_latched_regs_108_dpot_1869 : STD_LOGIC; 
  signal Source_latched_regs_109_dpot_1862 : STD_LOGIC; 
  signal Source_latched_regs_110_dpot_1855 : STD_LOGIC; 
  signal Source_latched_regs_127_dpot_1906 : STD_LOGIC; 
  signal Source_latched_regs_124_dpot_1899 : STD_LOGIC; 
  signal Source_latched_regs_125_dpot_1892 : STD_LOGIC; 
  signal Source_latched_regs_126_dpot_1885 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_12_dpot_1965 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_13_dpot_1958 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_14_dpot_1951 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_15_dpot_1942 : STD_LOGIC; 
  signal Source_latched_regs_4_dpot_1992 : STD_LOGIC; 
  signal Source_latched_regs_1_dpot_1985 : STD_LOGIC; 
  signal Source_latched_regs_2_dpot_1978 : STD_LOGIC; 
  signal Source_latched_regs_3_dpot_1971 : STD_LOGIC; 
  signal Source_latched_regs_35_dpot_2022 : STD_LOGIC; 
  signal Source_latched_regs_32_dpot_2015 : STD_LOGIC; 
  signal Source_latched_regs_33_dpot_2008 : STD_LOGIC; 
  signal Source_latched_regs_34_dpot_2001 : STD_LOGIC; 
  signal Source_clk_cnt_1_pack_4 : STD_LOGIC; 
  signal Source_latched_regs_75_dpot_2071 : STD_LOGIC; 
  signal Source_latched_regs_72_dpot_2064 : STD_LOGIC; 
  signal Source_latched_regs_73_dpot_2057 : STD_LOGIC; 
  signal Source_latched_regs_74_dpot_2050 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_89_dpot_2107 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_90_dpot_2100 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_91_dpot_2091 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_88_dpot_2084 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_93_dpot_2137 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_94_dpot_2130 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_95_dpot_2121 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_92_dpot_2114 : STD_LOGIC; 
  signal Source_latched_regs_115_dpot_2161 : STD_LOGIC; 
  signal Source_latched_regs_112_dpot_2154 : STD_LOGIC; 
  signal Source_latched_regs_113_dpot_2147 : STD_LOGIC; 
  signal Source_latched_regs_114_dpot_2140 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_125_dpot_2197 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_126_dpot_2190 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_127_dpot_2181 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_124_dpot_2174 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_6_pack_10 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_6_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_7_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_18_pack_4 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_18_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_2_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_5_pack_6 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_5_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_3_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_11_pack_8 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_11_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_4_Q : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_8_dpot_2266 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_9_dpot_2259 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_10_dpot_2252 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_11_dpot_2243 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_4_dpot_2296 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_5_dpot_2289 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_6_dpot_2282 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_7_dpot_2273 : STD_LOGIC; 
  signal Source_reg_cnt_1_pack_9 : STD_LOGIC; 
  signal Source_reg_cnt_1_dpot_2311 : STD_LOGIC; 
  signal Source_reg_cnt_0_dpot_2310 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_77_dpot_2367 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_78_dpot_2360 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_79_dpot_2351 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_76_dpot_2344 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_73_dpot_2397 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_74_dpot_2390 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_75_dpot_2381 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_72_dpot_2374 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_101_dpot_2427 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_102_dpot_2420 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_103_dpot_2411 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_100_dpot_2404 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_117_dpot_2457 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_118_dpot_2450 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_119_dpot_2441 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_116_dpot_2434 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_109_dpot_2487 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_110_dpot_2480 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_111_dpot_2471 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_108_dpot_2464 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_121_dpot_2517 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_122_dpot_2510 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_123_dpot_2501 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_120_dpot_2494 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_14_pack_10 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_14_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_15_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_8_pack_4 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_8_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_9_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_0_pack_6 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_0_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_10_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_13_pack_8 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_13_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_12_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_22_pack_10 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_22_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_23_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_16_pack_4 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_16_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_17_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_1_pack_6 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_1_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_20_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_19_pack_8 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_19_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_21_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_24_pack_2 : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_24_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_25_Q : STD_LOGIC; 
  signal Modulation_clk_sample_cnt_26_GND_22_o_MUX_524_o : STD_LOGIC; 
  signal Modulation_NEW_OUTPUT_pack_2 : STD_LOGIC; 
  signal Modulation_NEW_OUTPUT_rstpot_2644 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_17_dpot_2679 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_18_dpot_2672 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_19_dpot_2663 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_16_dpot_2656 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_25_dpot_2709 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_26_dpot_2702 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_27_dpot_2693 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_24_dpot_2686 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_29_dpot_2739 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_30_dpot_2732 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_31_dpot_2723 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_28_dpot_2716 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_12_rt_2762 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_13_rt_2756 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_14_rt_2751 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_15_rt_2744 : STD_LOGIC; 
  signal SPI_SOURCE_ATTENUATION_1_dpot_2787 : STD_LOGIC; 
  signal SPI_SOURCE_ATTENUATION_2_dpot_2782 : STD_LOGIC; 
  signal SPI_SOURCE_ATTENUATION_3_dpot_2775 : STD_LOGIC; 
  signal SPI_SOURCE_ATTENUATION_0_dpot_2770 : STD_LOGIC; 
  signal Source_Mcount_bit_cnt4 : STD_LOGIC; 
  signal Source_Mcount_bit_cnt5 : STD_LOGIC; 
  signal Source_bit_cnt_0_pack_15 : STD_LOGIC; 
  signal Source_Mcount_bit_cnt : STD_LOGIC; 
  signal Source_Mcount_bit_cnt1 : STD_LOGIC; 
  signal Source_bit_cnt_3_pack_17 : STD_LOGIC; 
  signal Source_Mcount_bit_cnt3 : STD_LOGIC; 
  signal Source_Mcount_bit_cnt2 : STD_LOGIC; 
  signal SPI_SPI_BUF_OUT_4_rt_2846 : STD_LOGIC; 
  signal SPI_SPI_BUF_OUT_5_rt_2841 : STD_LOGIC; 
  signal SPI_SPI_BUF_OUT_6_rt_2836 : STD_LOGIC; 
  signal SPI_SPI_BUF_OUT_7_rt_2829 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_81_dpot_2876 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_82_dpot_2869 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_83_dpot_2860 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_80_dpot_2853 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_74_rt_2896 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_75_rt_2889 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_72_rt_2884 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_73_rt_2879 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_78_rt_2918 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_79_rt_2911 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_76_rt_2906 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_77_rt_2901 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_110_rt_2940 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_111_rt_2933 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_108_rt_2928 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_109_rt_2923 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_106_rt_2962 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_107_rt_2955 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_104_rt_2950 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_105_rt_2945 : STD_LOGIC; 
  signal SOURCE_CLK_OBUF_2978 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_15_dpot1_2995 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_4_dpot1_3018 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_5_dpot1_3013 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_6_dpot1_3006 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_3_dpot1_3001 : STD_LOGIC; 
  signal SPI_MOD_ENABLE_rstpot_3023 : STD_LOGIC; 
  signal Modulation_sample_6_rt_3064 : STD_LOGIC; 
  signal Modulation_sample_7_rt_3057 : STD_LOGIC; 
  signal Modulation_sample_4_rt_3051 : STD_LOGIC; 
  signal Modulation_sample_5_rt_3046 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_33_dpot_3102 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_34_dpot_3095 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_35_dpot_3086 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_32_dpot_3079 : STD_LOGIC; 
  signal SPI_SOURCE_PORTSELECT_dpot_3109 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_10_rt_3129 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_11_rt_3122 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_8_rt_3117 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_9_rt_3112 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_48_rt_3161 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_49_rt_3155 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_50_rt_3150 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_51_rt_3143 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_85_dpot_3196 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_86_dpot_3189 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_87_dpot_3180 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_84_dpot_3173 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_97_dpot_3226 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_98_dpot_3219 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_99_dpot_3210 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_96_dpot_3203 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_84_rt_3259 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_85_rt_3253 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_86_rt_3248 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_87_rt_3241 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_113_dpot_3294 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_114_dpot_3287 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_115_dpot_3278 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_112_dpot_3271 : STD_LOGIC; 
  signal LO1_CLK_OBUF_3308 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_0_dpot1_3346 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_1_dpot1_3335 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_2_dpot1_3328 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_2_rt_3367 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_3_rt_3360 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_0_rt_3355 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_1_rt_3350 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_21_dpot_3399 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_22_dpot_3392 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_23_dpot_3383 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_20_dpot_3376 : STD_LOGIC; 
  signal SPI_SOURCE_ATTENUATION_4_dpot_3416 : STD_LOGIC; 
  signal SPI_SOURCE_ATTENUATION_5_dpot_3411 : STD_LOGIC; 
  signal SPI_SOURCE_ATTENUATION_6_dpot_3406 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_45_dpot_3460 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_46_dpot_3453 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_47_dpot_3444 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_44_dpot_3437 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_46_rt_3480 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_47_rt_3473 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_44_rt_3468 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_45_rt_3463 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_118_rt_3529 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_119_rt_3522 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_116_rt_3513 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_117_rt_3507 : STD_LOGIC; 
  signal SPI_n0487_inv_cepot_3638 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_8_pack_1 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_7_rt_3725 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_9_pack_3 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_8_rt_3720 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_10_pack_5 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_9_rt_3715 : STD_LOGIC; 
  signal SPI_SPI_mosi_buffer_10_rt_3708 : STD_LOGIC; 
  signal SPI_SPI_data_15_SPI_SPI_mosi_buffer_7_rt_3747 : STD_LOGIC; 
  signal SPI_SPI_data_15_SPI_SPI_mosi_buffer_8_rt_3742 : STD_LOGIC; 
  signal SPI_SPI_data_15_SPI_SPI_mosi_buffer_9_rt_3737 : STD_LOGIC; 
  signal SPI_SPI_data_15_SPI_SPI_mosi_buffer_10_rt_3730 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_8_dpot1_3769 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_9_dpot1_3764 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_10_dpot1_3757 : STD_LOGIC; 
  signal SPI_MOD_PHASE_INC_7_dpot1_3752 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_6_rt_3789 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_7_rt_3782 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_4_rt_3777 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_5_rt_3772 : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_8CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_14CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_13CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_8_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_13_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_14_Q : STD_LOGIC; 
  signal N87_INV_SPI_SPI_miso_buffer_11CLK : STD_LOGIC; 
  signal N87_INV_SPI_SPI_miso_buffer_10CLK : STD_LOGIC; 
  signal N87_INV_SPI_SPI_miso_buffer_9CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_10_pack_10 : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_10_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_11_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_9_pack_9 : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_9_Q : STD_LOGIC; 
  signal SPI_SPI_data_valid_0_rstpot_3858 : STD_LOGIC; 
  signal SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv : STD_LOGIC; 
  signal SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_15_Q : STD_LOGIC; 
  signal SPI_SPI_Mshreg_data_synced_2_3885 : STD_LOGIC; 
  signal Sync_AUX2_Mshreg_sync_line_1_3927 : STD_LOGIC; 
  signal Sync_AUX1_Mshreg_sync_line_1_3920 : STD_LOGIC; 
  signal Sync_NSS_Mshreg_sync_line_1_3913 : STD_LOGIC; 
  signal Sync_NSS_sync_line_1_rt_3963 : STD_LOGIC; 
  signal N102 : STD_LOGIC; 
  signal N101 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_2_dpot_3983 : STD_LOGIC; 
  signal N112 : STD_LOGIC; 
  signal N111 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_7_dpot_4014 : STD_LOGIC; 
  signal N110 : STD_LOGIC; 
  signal N109 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_6_dpot_4033 : STD_LOGIC; 
  signal N106 : STD_LOGIC; 
  signal N105 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_4_dpot_4052 : STD_LOGIC; 
  signal SPI_n0588_inv1_cepot_4072 : STD_LOGIC; 
  signal SPI_SOURCE_FILTER_0_pack_4 : STD_LOGIC; 
  signal SPI_SOURCE_FILTER_0_dpot_4080 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_24_rt_4134 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_25_rt_4128 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_26_rt_4123 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_27_rt_4116 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_49_dpot_4169 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_50_dpot_4162 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_51_dpot_4153 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_48_dpot_4146 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_61_dpot_4199 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_62_dpot_4192 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_63_dpot_4183 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_60_dpot_4176 : STD_LOGIC; 
  signal SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_3CLK : STD_LOGIC; 
  signal SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_2CLK : STD_LOGIC; 
  signal SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_1CLK : STD_LOGIC; 
  signal SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_0CLK : STD_LOGIC; 
  signal SPI_SPI_Mcount_bit_cnt : STD_LOGIC; 
  signal MCU_MISO_OBUFT_4224 : STD_LOGIC; 
  signal aux1_sync_aux2_sync_OR_8_o_inv : STD_LOGIC; 
  signal SPI_mod_data_LSB_5_dpot_4302 : STD_LOGIC; 
  signal SPI_mod_data_LSB_6_dpot_4294 : STD_LOGIC; 
  signal SPI_mod_data_LSB_7_dpot_4284 : STD_LOGIC; 
  signal SPI_mod_data_LSB_4_dpot_4277 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_57_dpot_4353 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_58_dpot_4346 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_59_dpot_4337 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_56_dpot_4330 : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_12CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_7CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_5CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_4CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_12_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_4_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_5_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_7_Q : STD_LOGIC; 
  signal SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_1_Q : STD_LOGIC; 
  signal SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_2_Q : STD_LOGIC; 
  signal SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_3_Q : STD_LOGIC; 
  signal SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_4_Q : STD_LOGIC; 
  signal SPI_interrupt_mask_4_dpot1_4426 : STD_LOGIC; 
  signal SPI_n0539_inv11_1_cepot_4437 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_5_pack_4 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_5_dpot1_4472 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_4_dpot1_4471 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_7_pack_6 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_7_dpot1_4463 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_6_dpot1_4462 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_9_pack_8 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_9_dpot1_4454 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_8_dpot1_4453 : STD_LOGIC; 
  signal SPI_Mcount_selected_register : STD_LOGIC; 
  signal SPI_Mcount_selected_register4 : STD_LOGIC; 
  signal N6_pack_7 : STD_LOGIC; 
  signal SPI_Mcount_selected_register2 : STD_LOGIC; 
  signal N104 : STD_LOGIC; 
  signal N103 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_3_dpot_4516 : STD_LOGIC; 
  signal N99 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_1_dpot_4558 : STD_LOGIC; 
  signal N100 : STD_LOGIC; 
  signal N108 : STD_LOGIC; 
  signal N107 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_5_dpot_4564 : STD_LOGIC; 
  signal N96 : STD_LOGIC; 
  signal N95 : STD_LOGIC; 
  signal Modulation_read_pos_10_write_pos_10_equal_9_o116_4583 : STD_LOGIC; 
  signal N98 : STD_LOGIC; 
  signal N97 : STD_LOGIC; 
  signal SPI_MOD_FIFO_DATA_0_dpot_4613 : STD_LOGIC; 
  signal SPI_word_cnt_1_pack_11 : STD_LOGIC; 
  signal SPI_word_cnt_3_pack_13 : STD_LOGIC; 
  signal SPI_SPI_Mcount_bit_cnt2_INV_SPI_SPI_miso_buffer_6CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_6_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_3CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_2CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_1CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_0CLK : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_2_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_3_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_0_Q : STD_LOGIC; 
  signal SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_1_Q : STD_LOGIC; 
  signal SPI_GND_17_o_INV_43_o_4716 : STD_LOGIC; 
  signal SPI_interrupt_mask_1_dpot1_4748 : STD_LOGIC; 
  signal SPI_interrupt_mask_2_dpot1_4734 : STD_LOGIC; 
  signal SPI_interrupt_mask_3_dpot1_4724 : STD_LOGIC; 
  signal SPI_Mcount_selected_register3 : STD_LOGIC; 
  signal SPI_mod_data_LSB_1_dpot_4834 : STD_LOGIC; 
  signal SPI_mod_data_LSB_2_dpot_4826 : STD_LOGIC; 
  signal SPI_mod_data_LSB_3_dpot_4816 : STD_LOGIC; 
  signal SPI_mod_data_LSB_0_dpot_4809 : STD_LOGIC; 
  signal SPI_LEDS_1_dpot1_4866 : STD_LOGIC; 
  signal SPI_LEDS_2_dpot1_4858 : STD_LOGIC; 
  signal SPI_LEDS_0_dpot1_4847 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_37_dpot_4897 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_38_dpot_4890 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_39_dpot_4881 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_36_dpot_4874 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_65_dpot_4927 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_66_dpot_4920 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_67_dpot_4911 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_64_dpot_4904 : STD_LOGIC; 
  signal Sync_SOURCE_LD_Mshreg_sync_line_2_4941 : STD_LOGIC; 
  signal Sync_AUX3_Mshreg_sync_line_2_4934 : STD_LOGIC; 
  signal source_unlocked : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_10_pack_5 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_10_dpot1_4982 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_1_dpot1_4981 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_3_pack_7 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_0_dpot1_4968 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_3_dpot1_4963 : STD_LOGIC; 
  signal SPI_MOD_FIFO_THRESHOLD_2_dpot1_4962 : STD_LOGIC; 
  signal SPI_mod_first_byte_pack_2 : STD_LOGIC; 
  signal SPI_mod_first_byte_glue_set : STD_LOGIC; 
  signal SPI_mod_table_write_glue_set : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_41_dpot_5049 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_42_dpot_5042 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_43_dpot_5033 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_40_dpot_5026 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_53_dpot_5079 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_54_dpot_5072 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_55_dpot_5063 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_52_dpot_5056 : STD_LOGIC; 
  signal Source_latched_regs_55_dpot_5105 : STD_LOGIC; 
  signal Source_latched_regs_56_dpot_5098 : STD_LOGIC; 
  signal Source_latched_regs_57_dpot_5083 : STD_LOGIC; 
  signal Source_latched_regs_63_dpot_5127 : STD_LOGIC; 
  signal Source_latched_regs_62_dpot_5116 : STD_LOGIC; 
  signal Modulation_OVERFLOW_glue_set_5154 : STD_LOGIC; 
  signal Modulation_UNDERFLOW_glue_set_5170 : STD_LOGIC; 
  signal Modulation_read_pos_4_rstpot_5235 : STD_LOGIC; 
  signal Modulation_read_pos_5_rstpot_5227 : STD_LOGIC; 
  signal Modulation_read_pos_6_rstpot_5219 : STD_LOGIC; 
  signal Modulation_read_pos_7_rstpot_5210 : STD_LOGIC; 
  signal SPI_mod_second_byte_glue_set : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_38_rt_5274 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_39_rt_5267 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_36_rt_5262 : STD_LOGIC; 
  signal SPI_mod_lookup_buffer_37_rt_5257 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_69_dpot_5306 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_70_dpot_5299 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_71_dpot_5290 : STD_LOGIC; 
  signal SPI_MOD_LOOKUP_DATA_68_dpot_5283 : STD_LOGIC; 
  signal Source_latched_regs_36_dpot_5340 : STD_LOGIC; 
  signal Source_latched_regs_37_dpot_5333 : STD_LOGIC; 
  signal Source_latched_regs_38_dpot_5318 : STD_LOGIC; 
  signal Source_latched_regs_71_dpot_5367 : STD_LOGIC; 
  signal Source_latched_regs_68_dpot_5360 : STD_LOGIC; 
  signal Source_latched_regs_69_dpot_5353 : STD_LOGIC; 
  signal Source_latched_regs_70_dpot_5346 : STD_LOGIC; 
  signal Modulation_read_pos_2_rstpot_5415 : STD_LOGIC; 
  signal Modulation_read_pos_3_rstpot_5406 : STD_LOGIC; 
  signal Modulation_read_pos_0_rstpot_5397 : STD_LOGIC; 
  signal Modulation_read_pos_1_rstpot_5389 : STD_LOGIC; 
  signal Modulation_read_pos_8_rstpot_5465 : STD_LOGIC; 
  signal Modulation_read_pos_9_rstpot_5457 : STD_LOGIC; 
  signal Modulation_read_pos_10_rstpot_5449 : STD_LOGIC; 
  signal LEDS_4_OBUF_5482 : STD_LOGIC; 
  signal Source_latched_regs_61_dpot_5507 : STD_LOGIC; 
  signal Source_latched_regs_58_dpot_5500 : STD_LOGIC; 
  signal Source_latched_regs_59_dpot_5493 : STD_LOGIC; 
  signal Source_latched_regs_60_dpot_5486 : STD_LOGIC; 
  signal Source_latched_regs_67_dpot_5537 : STD_LOGIC; 
  signal Source_latched_regs_64_dpot_5530 : STD_LOGIC; 
  signal Source_latched_regs_65_dpot_5523 : STD_LOGIC; 
  signal Source_latched_regs_66_dpot_5516 : STD_LOGIC; 
  signal Inst_ResetDelay_clk_cnt_1_pack_1 : STD_LOGIC; 
  signal Source_latched_regs_42_dpot_5578 : STD_LOGIC; 
  signal Source_latched_regs_39_dpot_5571 : STD_LOGIC; 
  signal Source_latched_regs_40_dpot_5564 : STD_LOGIC; 
  signal Source_latched_regs_41_dpot_5557 : STD_LOGIC; 
  signal Source_latched_regs_54_dpot_5608 : STD_LOGIC; 
  signal Source_latched_regs_51_dpot_5601 : STD_LOGIC; 
  signal Source_latched_regs_52_dpot_5594 : STD_LOGIC; 
  signal Source_latched_regs_53_dpot_5587 : STD_LOGIC; 
  signal Source_latched_regs_50_dpot_5638 : STD_LOGIC; 
  signal Source_latched_regs_47_dpot_5631 : STD_LOGIC; 
  signal Source_latched_regs_48_dpot_5624 : STD_LOGIC; 
  signal Source_latched_regs_49_dpot_5617 : STD_LOGIC; 
  signal Source_latched_regs_46_dpot_5668 : STD_LOGIC; 
  signal Source_latched_regs_43_dpot_5661 : STD_LOGIC; 
  signal Source_latched_regs_44_dpot_5654 : STD_LOGIC; 
  signal Source_latched_regs_45_dpot_5647 : STD_LOGIC; 
  signal N4_pack_16 : STD_LOGIC; 
  signal Inst_ResetDelay_clk_cnt_3_pack_7 : STD_LOGIC; 
  signal N91_pack_9 : STD_LOGIC; 
  signal GND : STD_LOGIC; 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_0_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_1_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_10_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_11_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_12_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_13_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_14_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_15_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_16_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_17_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_18_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_19_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_2_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_20_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_21_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_22_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_23_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_24_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_25_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_26_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_27_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_28_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_29_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_3_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_30_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_31_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_4_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_5_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_6_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_7_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_8_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_9_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_10_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_11_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_12_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_13_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_14_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_15_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_16_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_17_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_18_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_19_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_20_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_21_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_22_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_23_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_24_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_25_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_26_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_27_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_28_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_29_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_30_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_31_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_8_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_9_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_0_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_1_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_2_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_3_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_0_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_1_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_2_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_3_UNCONNECTED : STD_LOGIC;
 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_CO_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_DI_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_DI_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_S_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_S_3_UNCONNECTED : STD_LOGIC; 
  signal VCC : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_DI_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_DI_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_O_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_S_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_7_D5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_cy_3_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_cy_3_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_cy_3_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_8_C5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_9_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_MIX1_EN_OBUF_1_2_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_3_D5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_cy_7_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_cy_7_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_cy_7_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_4_C5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_5_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_6_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_CO_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_DI_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_DI_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_O_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_read_pos_xor_10_S_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_2_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_29_D5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_30_C5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_31_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_32_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_25_D5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_26_C5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_27_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_28_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_DI_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_DI_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_O_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_S_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_23_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_24_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_18_D5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_cy_3_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_cy_3_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_cy_3_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_19_C5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_20_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_MIX1_EN_OBUF_1_3_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_14_D5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_cy_7_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_cy_7_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_cy_7_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_15_C5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_16_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_17_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_CO_0_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_CO_1_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_CO_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_CO_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_DI_2_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_DI_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_O_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_Modulation_Mcount_write_pos_xor_10_S_3_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_12_B5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_REF_SCLK_OBUF_1_13_A5LUT_O_UNCONNECTED : STD_LOGIC; 
  signal NLW_SP6_BUFIO_INSERT_ML_BUFIO2_0_IOCLK_UNCONNECTED : STD_LOGIC; 
  signal NLW_SP6_BUFIO_INSERT_ML_BUFIO2_0_SERDESSTROBE_UNCONNECTED : STD_LOGIC; 
  signal NLW_SPI_SPI_Mshreg_data_synced_2_Q15_UNCONNECTED : STD_LOGIC; 
  signal NLW_Sync_NSS_Mshreg_sync_line_1_Q15_UNCONNECTED : STD_LOGIC; 
  signal NLW_Sync_AUX1_Mshreg_sync_line_1_Q15_UNCONNECTED : STD_LOGIC; 
  signal NLW_Sync_AUX2_Mshreg_sync_line_1_Q15_UNCONNECTED : STD_LOGIC; 
  signal NLW_Sync_AUX3_Mshreg_sync_line_2_Q15_UNCONNECTED : STD_LOGIC; 
  signal NLW_Sync_SOURCE_LD_Mshreg_sync_line_2_Q15_UNCONNECTED : STD_LOGIC; 
  signal Modulation_clk_sample_cnt : STD_LOGIC_VECTOR ( 26 downto 0 ); 
  signal Sync_AUX3_sync_line : STD_LOGIC_VECTOR ( 2 downto 2 ); 
  signal SPI_SPI_bit_cnt : STD_LOGIC_VECTOR ( 3 downto 0 ); 
  signal SPI_MOD_FIFO_THRESHOLD : STD_LOGIC_VECTOR ( 10 downto 0 ); 
  signal Sync_AUX1_sync_line : STD_LOGIC_VECTOR ( 2 downto 1 ); 
  signal Source_clk_cnt : STD_LOGIC_VECTOR ( 2 downto 0 ); 
  signal Inst_ResetDelay_clk_cnt : STD_LOGIC_VECTOR ( 6 downto 0 ); 
  signal Sync_AUX2_sync_line : STD_LOGIC_VECTOR ( 2 downto 1 ); 
  signal Source_bit_cnt : STD_LOGIC_VECTOR ( 5 downto 0 ); 
  signal SPI_word_cnt : STD_LOGIC_VECTOR ( 4 downto 0 ); 
  signal SPI_SPI_BUF_OUT : STD_LOGIC_VECTOR ( 15 downto 0 ); 
  signal SPI_selected_register : STD_LOGIC_VECTOR ( 4 downto 0 ); 
  signal SPI_SPI_miso_buffer : STD_LOGIC_VECTOR ( 14 downto 0 ); 
  signal mod_lookup_read_data : STD_LOGIC_VECTOR ( 134 downto 0 ); 
  signal SPI_MOD_LOOKUP_INDEX : STD_LOGIC_VECTOR ( 7 downto 0 ); 
  signal SPI_MOD_LOOKUP_DATA : STD_LOGIC_VECTOR ( 143 downto 0 ); 
  signal Modulation_OUTPUT_SAMPLE : STD_LOGIC_VECTOR ( 7 downto 0 ); 
  signal Modulation_write_pos : STD_LOGIC_VECTOR ( 10 downto 0 ); 
  signal Modulation_read_pos : STD_LOGIC_VECTOR ( 10 downto 0 ); 
  signal Modulation_sample : STD_LOGIC_VECTOR ( 7 downto 0 ); 
  signal SPI_MOD_FIFO_DATA : STD_LOGIC_VECTOR ( 7 downto 0 ); 
  signal SPI_MOD_PHASE_INC : STD_LOGIC_VECTOR ( 15 downto 0 ); 
  signal SPI_SOURCE_FILTER : STD_LOGIC_VECTOR ( 1 downto 0 ); 
  signal SPI_LEDS : STD_LOGIC_VECTOR ( 2 downto 0 ); 
  signal SPI_mod_data_LSB : STD_LOGIC_VECTOR ( 7 downto 0 ); 
  signal Sync_SOURCE_LD_sync_line : STD_LOGIC_VECTOR ( 2 downto 2 ); 
  signal Sync_NSS_sync_line : STD_LOGIC_VECTOR ( 2 downto 1 ); 
  signal SPI_interrupt_mask : STD_LOGIC_VECTOR ( 4 downto 1 ); 
  signal SPI_interrupt_status : STD_LOGIC_VECTOR ( 4 downto 1 ); 
  signal Source_latched_regs : STD_LOGIC_VECTOR ( 127 downto 0 ); 
  signal SPI_SOURCE_ATTENUATION : STD_LOGIC_VECTOR ( 6 downto 0 ); 
  signal SPI_mod_lookup_buffer : STD_LOGIC_VECTOR ( 127 downto 0 ); 
  signal SPI_SPI_data : STD_LOGIC_VECTOR ( 15 downto 0 ); 
  signal SPI_SPI_mosi_buffer : STD_LOGIC_VECTOR ( 14 downto 0 ); 
  signal mod_attenuator : STD_LOGIC_VECTOR ( 6 downto 0 ); 
  signal Source_reg_cnt : STD_LOGIC_VECTOR ( 1 downto 0 ); 
  signal Modulation_GND_22_o_GND_22_o_sub_1_OUT : STD_LOGIC_VECTOR ( 10 downto 0 ); 
  signal Modulation_Mcount_read_pos_lut : STD_LOGIC_VECTOR ( 0 downto 0 ); 
  signal Modulation_Mcount_write_pos_lut : STD_LOGIC_VECTOR ( 0 downto 0 ); 
  signal Modulation_Result : STD_LOGIC_VECTOR ( 10 downto 0 ); 
  signal Source_Result : STD_LOGIC_VECTOR ( 2 downto 0 ); 
  signal SPI_Result : STD_LOGIC_VECTOR ( 4 downto 0 ); 
  signal Result : STD_LOGIC_VECTOR ( 6 downto 0 ); 
begin
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTAINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y22",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCEINV : 
X_BUF
    generic map(
      LOC => "RAMB8_X0Y22",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWRENINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y22",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEAINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y22",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDENINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y22",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRSTINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y22",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram : X_RAMB8BWER
    generic map(
      DATA_WIDTH_A => 36,
      DATA_WIDTH_B => 36,
      DOA_REG => 0,
      DOB_REG => 0,
      EN_RSTRAM_A => FALSE,
      EN_RSTRAM_B => FALSE,
      RAM_MODE => "SDP",
      RST_PRIORITY_A => "CE",
      RST_PRIORITY_B => "CE",
      RSTTYPE => "SYNC",
      WRITE_MODE_A => "WRITE_FIRST",
      WRITE_MODE_B => "WRITE_FIRST",
      INIT_FILE => "NONE",
      SIM_COLLISION_CHECK => "ALL",
      INITP_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_08 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_09 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_10 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_11 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_12 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_13 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_14 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_15 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_16 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_17 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_18 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_19 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_A => X"00000",
      INIT_B => X"00000",
      SRVAL_A => X"00000",
      SRVAL_B => X"00000",
      LOC => "RAMB8_X0Y22"
    )
    port map (
      RSTBRST => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT,
      ENBRDEN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT,
      REGCEA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT,
      ENAWREN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT,
      CLKAWRCLK => clk_pll,
      CLKBRDCLK => clk_pll,
      REGCEBREGCE => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT,
      RSTA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT,
      WEAWEL(1) => SPI_mod_table_write_5972,
      WEAWEL(0) => SPI_mod_table_write_5972,
      WEBWEU(1) => SPI_mod_table_write_5972,
      WEBWEU(0) => SPI_mod_table_write_5972,
      ADDRAWRADDR(12) => SPI_MOD_LOOKUP_INDEX_7_0,
      ADDRAWRADDR(11) => SPI_MOD_LOOKUP_INDEX_6_0,
      ADDRAWRADDR(10) => SPI_MOD_LOOKUP_INDEX_5_0,
      ADDRAWRADDR(9) => SPI_MOD_LOOKUP_INDEX_4_0,
      ADDRAWRADDR(8) => SPI_MOD_LOOKUP_INDEX(3),
      ADDRAWRADDR(7) => SPI_MOD_LOOKUP_INDEX(2),
      ADDRAWRADDR(6) => SPI_MOD_LOOKUP_INDEX(1),
      ADDRAWRADDR(5) => SPI_MOD_LOOKUP_INDEX(0),
      ADDRAWRADDR(4) => GND,
      ADDRAWRADDR(3) => GND,
      ADDRAWRADDR(2) => GND,
      ADDRAWRADDR(1) => GND,
      ADDRAWRADDR(0) => GND,
      DIPBDIP(1) => SPI_MOD_LOOKUP_DATA(143),
      DIPBDIP(0) => SPI_MOD_LOOKUP_DATA_134_0,
      DIBDI(15) => SPI_MOD_LOOKUP_DATA(142),
      DIBDI(14) => SPI_MOD_LOOKUP_DATA(141),
      DIBDI(13) => SPI_MOD_LOOKUP_DATA(140),
      DIBDI(12) => SPI_MOD_LOOKUP_DATA(139),
      DIBDI(11) => SPI_MOD_LOOKUP_DATA(138),
      DIBDI(10) => SPI_MOD_LOOKUP_DATA(137),
      DIBDI(9) => SPI_MOD_LOOKUP_DATA(136),
      DIBDI(8) => SPI_MOD_LOOKUP_DATA_135_0,
      DIBDI(7) => SPI_MOD_LOOKUP_DATA_133_0,
      DIBDI(6) => SPI_MOD_LOOKUP_DATA_132_0,
      DIBDI(5) => SPI_MOD_LOOKUP_DATA(131),
      DIBDI(4) => SPI_MOD_LOOKUP_DATA(130),
      DIBDI(3) => SPI_MOD_LOOKUP_DATA(129),
      DIBDI(2) => SPI_MOD_LOOKUP_DATA(128),
      DIBDI(1) => SPI_MOD_LOOKUP_DATA(127),
      DIBDI(0) => SPI_MOD_LOOKUP_DATA(126),
      DIADI(15) => SPI_MOD_LOOKUP_DATA(124),
      DIADI(14) => SPI_MOD_LOOKUP_DATA(123),
      DIADI(13) => SPI_MOD_LOOKUP_DATA(122),
      DIADI(12) => SPI_MOD_LOOKUP_DATA(121),
      DIADI(11) => SPI_MOD_LOOKUP_DATA(120),
      DIADI(10) => SPI_MOD_LOOKUP_DATA(119),
      DIADI(9) => SPI_MOD_LOOKUP_DATA(118),
      DIADI(8) => SPI_MOD_LOOKUP_DATA(117),
      DIADI(7) => SPI_MOD_LOOKUP_DATA(115),
      DIADI(6) => SPI_MOD_LOOKUP_DATA(114),
      DIADI(5) => SPI_MOD_LOOKUP_DATA(113),
      DIADI(4) => SPI_MOD_LOOKUP_DATA(112),
      DIADI(3) => SPI_MOD_LOOKUP_DATA(111),
      DIADI(2) => SPI_MOD_LOOKUP_DATA(110),
      DIADI(1) => SPI_MOD_LOOKUP_DATA(109),
      DIADI(0) => SPI_MOD_LOOKUP_DATA(108),
      ADDRBRDADDR(12) => Modulation_OUTPUT_SAMPLE_7_0,
      ADDRBRDADDR(11) => Modulation_OUTPUT_SAMPLE_6_0,
      ADDRBRDADDR(10) => Modulation_OUTPUT_SAMPLE_5_0,
      ADDRBRDADDR(9) => Modulation_OUTPUT_SAMPLE_4_0,
      ADDRBRDADDR(8) => Modulation_OUTPUT_SAMPLE(3),
      ADDRBRDADDR(7) => Modulation_OUTPUT_SAMPLE(2),
      ADDRBRDADDR(6) => Modulation_OUTPUT_SAMPLE(1),
      ADDRBRDADDR(5) => Modulation_OUTPUT_SAMPLE(0),
      ADDRBRDADDR(4) => GND,
      ADDRBRDADDR(3) => GND,
      ADDRBRDADDR(2) => GND,
      ADDRBRDADDR(1) => GND,
      ADDRBRDADDR(0) => GND,
      DIPADIP(1) => SPI_MOD_LOOKUP_DATA(125),
      DIPADIP(0) => SPI_MOD_LOOKUP_DATA(116),
      DOADO(15) => mod_lookup_read_data(124),
      DOADO(14) => mod_lookup_read_data(123),
      DOADO(13) => mod_lookup_read_data(122),
      DOADO(12) => mod_lookup_read_data(121),
      DOADO(11) => mod_lookup_read_data(120),
      DOADO(10) => mod_lookup_read_data(119),
      DOADO(9) => mod_lookup_read_data(118),
      DOADO(8) => mod_lookup_read_data(117),
      DOADO(7) => mod_lookup_read_data(115),
      DOADO(6) => mod_lookup_read_data(114),
      DOADO(5) => mod_lookup_read_data(113),
      DOADO(4) => mod_lookup_read_data(112),
      DOADO(3) => mod_lookup_read_data(111),
      DOADO(2) => mod_lookup_read_data(110),
      DOADO(1) => mod_lookup_read_data(109),
      DOADO(0) => mod_lookup_read_data(108),
      DOPADOP(1) => mod_lookup_read_data(125),
      DOPADOP(0) => mod_lookup_read_data(116),
      DOPBDOP(1) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOPBDOP1,
      DOPBDOP(0) => mod_lookup_read_data(134),
      DOBDO(15) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO15,
      DOBDO(14) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO14,
      DOBDO(13) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO13,
      DOBDO(12) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO12,
      DOBDO(11) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO11,
      DOBDO(10) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO10,
      DOBDO(9) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO9,
      DOBDO(8) => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_3_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_DOBDO8,
      DOBDO(7) => mod_lookup_read_data(133),
      DOBDO(6) => mod_lookup_read_data(132),
      DOBDO(5) => mod_lookup_read_data(131),
      DOBDO(4) => mod_lookup_read_data(130),
      DOBDO(3) => mod_lookup_read_data(129),
      DOBDO(2) => mod_lookup_read_data(128),
      DOBDO(1) => mod_lookup_read_data(127),
      DOBDO(0) => mod_lookup_read_data(126)
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTAINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y19",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCEINV : 
X_BUF
    generic map(
      LOC => "RAMB8_X0Y19",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWRENINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y19",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEAINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y19",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDENINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y19",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRSTINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y19",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram : X_RAMB8BWER
    generic map(
      DATA_WIDTH_A => 36,
      DATA_WIDTH_B => 36,
      DOA_REG => 0,
      DOB_REG => 0,
      EN_RSTRAM_A => FALSE,
      EN_RSTRAM_B => FALSE,
      RAM_MODE => "SDP",
      RST_PRIORITY_A => "CE",
      RST_PRIORITY_B => "CE",
      RSTTYPE => "SYNC",
      WRITE_MODE_A => "WRITE_FIRST",
      WRITE_MODE_B => "WRITE_FIRST",
      INIT_FILE => "NONE",
      SIM_COLLISION_CHECK => "ALL",
      INITP_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_08 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_09 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_10 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_11 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_12 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_13 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_14 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_15 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_16 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_17 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_18 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_19 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_A => X"00000",
      INIT_B => X"00000",
      SRVAL_A => X"00000",
      SRVAL_B => X"00000",
      LOC => "RAMB8_X0Y19"
    )
    port map (
      RSTBRST => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT,
      ENBRDEN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT,
      REGCEA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT,
      ENAWREN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT,
      CLKAWRCLK => clk_pll,
      CLKBRDCLK => clk_pll,
      REGCEBREGCE => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT,
      RSTA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_2_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT,
      WEAWEL(1) => SPI_mod_table_write_5972,
      WEAWEL(0) => SPI_mod_table_write_5972,
      WEBWEU(1) => SPI_mod_table_write_5972,
      WEBWEU(0) => SPI_mod_table_write_5972,
      ADDRAWRADDR(12) => SPI_MOD_LOOKUP_INDEX_7_0,
      ADDRAWRADDR(11) => SPI_MOD_LOOKUP_INDEX_6_0,
      ADDRAWRADDR(10) => SPI_MOD_LOOKUP_INDEX_5_0,
      ADDRAWRADDR(9) => SPI_MOD_LOOKUP_INDEX_4_0,
      ADDRAWRADDR(8) => SPI_MOD_LOOKUP_INDEX(3),
      ADDRAWRADDR(7) => SPI_MOD_LOOKUP_INDEX(2),
      ADDRAWRADDR(6) => SPI_MOD_LOOKUP_INDEX(1),
      ADDRAWRADDR(5) => SPI_MOD_LOOKUP_INDEX(0),
      ADDRAWRADDR(4) => GND,
      ADDRAWRADDR(3) => GND,
      ADDRAWRADDR(2) => GND,
      ADDRAWRADDR(1) => GND,
      ADDRAWRADDR(0) => GND,
      DIPBDIP(1) => SPI_MOD_LOOKUP_DATA(107),
      DIPBDIP(0) => SPI_MOD_LOOKUP_DATA(98),
      DIBDI(15) => SPI_MOD_LOOKUP_DATA(106),
      DIBDI(14) => SPI_MOD_LOOKUP_DATA(105),
      DIBDI(13) => SPI_MOD_LOOKUP_DATA(104),
      DIBDI(12) => SPI_MOD_LOOKUP_DATA(103),
      DIBDI(11) => SPI_MOD_LOOKUP_DATA(102),
      DIBDI(10) => SPI_MOD_LOOKUP_DATA(101),
      DIBDI(9) => SPI_MOD_LOOKUP_DATA(100),
      DIBDI(8) => SPI_MOD_LOOKUP_DATA(99),
      DIBDI(7) => SPI_MOD_LOOKUP_DATA(97),
      DIBDI(6) => SPI_MOD_LOOKUP_DATA(96),
      DIBDI(5) => SPI_MOD_LOOKUP_DATA(95),
      DIBDI(4) => SPI_MOD_LOOKUP_DATA(94),
      DIBDI(3) => SPI_MOD_LOOKUP_DATA(93),
      DIBDI(2) => SPI_MOD_LOOKUP_DATA(92),
      DIBDI(1) => SPI_MOD_LOOKUP_DATA(91),
      DIBDI(0) => SPI_MOD_LOOKUP_DATA(90),
      DIADI(15) => SPI_MOD_LOOKUP_DATA(88),
      DIADI(14) => SPI_MOD_LOOKUP_DATA(87),
      DIADI(13) => SPI_MOD_LOOKUP_DATA(86),
      DIADI(12) => SPI_MOD_LOOKUP_DATA(85),
      DIADI(11) => SPI_MOD_LOOKUP_DATA(84),
      DIADI(10) => SPI_MOD_LOOKUP_DATA(83),
      DIADI(9) => SPI_MOD_LOOKUP_DATA(82),
      DIADI(8) => SPI_MOD_LOOKUP_DATA(81),
      DIADI(7) => SPI_MOD_LOOKUP_DATA(79),
      DIADI(6) => SPI_MOD_LOOKUP_DATA(78),
      DIADI(5) => SPI_MOD_LOOKUP_DATA(77),
      DIADI(4) => SPI_MOD_LOOKUP_DATA(76),
      DIADI(3) => SPI_MOD_LOOKUP_DATA(75),
      DIADI(2) => SPI_MOD_LOOKUP_DATA(74),
      DIADI(1) => SPI_MOD_LOOKUP_DATA(73),
      DIADI(0) => SPI_MOD_LOOKUP_DATA(72),
      ADDRBRDADDR(12) => Modulation_OUTPUT_SAMPLE_7_0,
      ADDRBRDADDR(11) => Modulation_OUTPUT_SAMPLE_6_0,
      ADDRBRDADDR(10) => Modulation_OUTPUT_SAMPLE_5_0,
      ADDRBRDADDR(9) => Modulation_OUTPUT_SAMPLE_4_0,
      ADDRBRDADDR(8) => Modulation_OUTPUT_SAMPLE(3),
      ADDRBRDADDR(7) => Modulation_OUTPUT_SAMPLE(2),
      ADDRBRDADDR(6) => Modulation_OUTPUT_SAMPLE(1),
      ADDRBRDADDR(5) => Modulation_OUTPUT_SAMPLE(0),
      ADDRBRDADDR(4) => GND,
      ADDRBRDADDR(3) => GND,
      ADDRBRDADDR(2) => GND,
      ADDRBRDADDR(1) => GND,
      ADDRBRDADDR(0) => GND,
      DIPADIP(1) => SPI_MOD_LOOKUP_DATA(89),
      DIPADIP(0) => SPI_MOD_LOOKUP_DATA(80),
      DOADO(15) => mod_lookup_read_data(88),
      DOADO(14) => mod_lookup_read_data(87),
      DOADO(13) => mod_lookup_read_data(86),
      DOADO(12) => mod_lookup_read_data(85),
      DOADO(11) => mod_lookup_read_data(84),
      DOADO(10) => mod_lookup_read_data(83),
      DOADO(9) => mod_lookup_read_data(82),
      DOADO(8) => mod_lookup_read_data(81),
      DOADO(7) => mod_lookup_read_data(79),
      DOADO(6) => mod_lookup_read_data(78),
      DOADO(5) => mod_lookup_read_data(77),
      DOADO(4) => mod_lookup_read_data(76),
      DOADO(3) => mod_lookup_read_data(75),
      DOADO(2) => mod_lookup_read_data(74),
      DOADO(1) => mod_lookup_read_data(73),
      DOADO(0) => mod_lookup_read_data(72),
      DOPADOP(1) => mod_lookup_read_data(89),
      DOPADOP(0) => mod_lookup_read_data(80),
      DOPBDOP(1) => mod_lookup_read_data(107),
      DOPBDOP(0) => mod_lookup_read_data(98),
      DOBDO(15) => mod_lookup_read_data(106),
      DOBDO(14) => mod_lookup_read_data(105),
      DOBDO(13) => mod_lookup_read_data(104),
      DOBDO(12) => mod_lookup_read_data(103),
      DOBDO(11) => mod_lookup_read_data(102),
      DOBDO(10) => mod_lookup_read_data(101),
      DOBDO(9) => mod_lookup_read_data(100),
      DOBDO(8) => mod_lookup_read_data(99),
      DOBDO(7) => mod_lookup_read_data(97),
      DOBDO(6) => mod_lookup_read_data(96),
      DOBDO(5) => mod_lookup_read_data(95),
      DOBDO(4) => mod_lookup_read_data(94),
      DOBDO(3) => mod_lookup_read_data(93),
      DOBDO(2) => mod_lookup_read_data(92),
      DOBDO(1) => mod_lookup_read_data(91),
      DOBDO(0) => mod_lookup_read_data(90)
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTAINV : X_BUF
    generic map(
      LOC => "RAMB8_X1Y18",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCEINV : 
X_BUF
    generic map(
      LOC => "RAMB8_X1Y18",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWRENINV : X_BUF
    generic map(
      LOC => "RAMB8_X1Y18",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEAINV : X_BUF
    generic map(
      LOC => "RAMB8_X1Y18",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDENINV : X_BUF
    generic map(
      LOC => "RAMB8_X1Y18",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRSTINV : X_BUF
    generic map(
      LOC => "RAMB8_X1Y18",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram : X_RAMB8BWER
    generic map(
      DATA_WIDTH_A => 36,
      DATA_WIDTH_B => 36,
      DOA_REG => 0,
      DOB_REG => 0,
      EN_RSTRAM_A => FALSE,
      EN_RSTRAM_B => FALSE,
      RAM_MODE => "SDP",
      RST_PRIORITY_A => "CE",
      RST_PRIORITY_B => "CE",
      RSTTYPE => "SYNC",
      WRITE_MODE_A => "WRITE_FIRST",
      WRITE_MODE_B => "WRITE_FIRST",
      INIT_FILE => "NONE",
      SIM_COLLISION_CHECK => "ALL",
      INITP_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_08 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_09 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_10 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_11 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_12 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_13 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_14 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_15 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_16 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_17 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_18 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_19 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_A => X"00000",
      INIT_B => X"00000",
      SRVAL_A => X"00000",
      SRVAL_B => X"00000",
      LOC => "RAMB8_X1Y18"
    )
    port map (
      RSTBRST => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT,
      ENBRDEN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT,
      REGCEA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT,
      ENAWREN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT,
      CLKAWRCLK => clk_pll,
      CLKBRDCLK => clk_pll,
      REGCEBREGCE => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT,
      RSTA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_1_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT,
      WEAWEL(1) => SPI_mod_table_write_5972,
      WEAWEL(0) => SPI_mod_table_write_5972,
      WEBWEU(1) => SPI_mod_table_write_5972,
      WEBWEU(0) => SPI_mod_table_write_5972,
      ADDRAWRADDR(12) => SPI_MOD_LOOKUP_INDEX_7_0,
      ADDRAWRADDR(11) => SPI_MOD_LOOKUP_INDEX_6_0,
      ADDRAWRADDR(10) => SPI_MOD_LOOKUP_INDEX_5_0,
      ADDRAWRADDR(9) => SPI_MOD_LOOKUP_INDEX_4_0,
      ADDRAWRADDR(8) => SPI_MOD_LOOKUP_INDEX(3),
      ADDRAWRADDR(7) => SPI_MOD_LOOKUP_INDEX(2),
      ADDRAWRADDR(6) => SPI_MOD_LOOKUP_INDEX(1),
      ADDRAWRADDR(5) => SPI_MOD_LOOKUP_INDEX(0),
      ADDRAWRADDR(4) => GND,
      ADDRAWRADDR(3) => GND,
      ADDRAWRADDR(2) => GND,
      ADDRAWRADDR(1) => GND,
      ADDRAWRADDR(0) => GND,
      DIPBDIP(1) => SPI_MOD_LOOKUP_DATA(71),
      DIPBDIP(0) => SPI_MOD_LOOKUP_DATA(62),
      DIBDI(15) => SPI_MOD_LOOKUP_DATA(70),
      DIBDI(14) => SPI_MOD_LOOKUP_DATA(69),
      DIBDI(13) => SPI_MOD_LOOKUP_DATA(68),
      DIBDI(12) => SPI_MOD_LOOKUP_DATA(67),
      DIBDI(11) => SPI_MOD_LOOKUP_DATA(66),
      DIBDI(10) => SPI_MOD_LOOKUP_DATA(65),
      DIBDI(9) => SPI_MOD_LOOKUP_DATA(64),
      DIBDI(8) => SPI_MOD_LOOKUP_DATA(63),
      DIBDI(7) => SPI_MOD_LOOKUP_DATA(61),
      DIBDI(6) => SPI_MOD_LOOKUP_DATA(60),
      DIBDI(5) => SPI_MOD_LOOKUP_DATA(59),
      DIBDI(4) => SPI_MOD_LOOKUP_DATA(58),
      DIBDI(3) => SPI_MOD_LOOKUP_DATA(57),
      DIBDI(2) => SPI_MOD_LOOKUP_DATA(56),
      DIBDI(1) => SPI_MOD_LOOKUP_DATA(55),
      DIBDI(0) => SPI_MOD_LOOKUP_DATA(54),
      DIADI(15) => SPI_MOD_LOOKUP_DATA(52),
      DIADI(14) => SPI_MOD_LOOKUP_DATA(51),
      DIADI(13) => SPI_MOD_LOOKUP_DATA(50),
      DIADI(12) => SPI_MOD_LOOKUP_DATA(49),
      DIADI(11) => SPI_MOD_LOOKUP_DATA(48),
      DIADI(10) => SPI_MOD_LOOKUP_DATA(47),
      DIADI(9) => SPI_MOD_LOOKUP_DATA(46),
      DIADI(8) => SPI_MOD_LOOKUP_DATA(45),
      DIADI(7) => SPI_MOD_LOOKUP_DATA(43),
      DIADI(6) => SPI_MOD_LOOKUP_DATA(42),
      DIADI(5) => SPI_MOD_LOOKUP_DATA(41),
      DIADI(4) => SPI_MOD_LOOKUP_DATA(40),
      DIADI(3) => SPI_MOD_LOOKUP_DATA(39),
      DIADI(2) => SPI_MOD_LOOKUP_DATA(38),
      DIADI(1) => SPI_MOD_LOOKUP_DATA(37),
      DIADI(0) => SPI_MOD_LOOKUP_DATA(36),
      ADDRBRDADDR(12) => Modulation_OUTPUT_SAMPLE_7_0,
      ADDRBRDADDR(11) => Modulation_OUTPUT_SAMPLE_6_0,
      ADDRBRDADDR(10) => Modulation_OUTPUT_SAMPLE_5_0,
      ADDRBRDADDR(9) => Modulation_OUTPUT_SAMPLE_4_0,
      ADDRBRDADDR(8) => Modulation_OUTPUT_SAMPLE(3),
      ADDRBRDADDR(7) => Modulation_OUTPUT_SAMPLE(2),
      ADDRBRDADDR(6) => Modulation_OUTPUT_SAMPLE(1),
      ADDRBRDADDR(5) => Modulation_OUTPUT_SAMPLE(0),
      ADDRBRDADDR(4) => GND,
      ADDRBRDADDR(3) => GND,
      ADDRBRDADDR(2) => GND,
      ADDRBRDADDR(1) => GND,
      ADDRBRDADDR(0) => GND,
      DIPADIP(1) => SPI_MOD_LOOKUP_DATA(53),
      DIPADIP(0) => SPI_MOD_LOOKUP_DATA(44),
      DOADO(15) => mod_lookup_read_data(52),
      DOADO(14) => mod_lookup_read_data(51),
      DOADO(13) => mod_lookup_read_data(50),
      DOADO(12) => mod_lookup_read_data(49),
      DOADO(11) => mod_lookup_read_data(48),
      DOADO(10) => mod_lookup_read_data(47),
      DOADO(9) => mod_lookup_read_data(46),
      DOADO(8) => mod_lookup_read_data(45),
      DOADO(7) => mod_lookup_read_data(43),
      DOADO(6) => mod_lookup_read_data(42),
      DOADO(5) => mod_lookup_read_data(41),
      DOADO(4) => mod_lookup_read_data(40),
      DOADO(3) => mod_lookup_read_data(39),
      DOADO(2) => mod_lookup_read_data(38),
      DOADO(1) => mod_lookup_read_data(37),
      DOADO(0) => mod_lookup_read_data(36),
      DOPADOP(1) => mod_lookup_read_data(53),
      DOPADOP(0) => mod_lookup_read_data(44),
      DOPBDOP(1) => mod_lookup_read_data(71),
      DOPBDOP(0) => mod_lookup_read_data(62),
      DOBDO(15) => mod_lookup_read_data(70),
      DOBDO(14) => mod_lookup_read_data(69),
      DOBDO(13) => mod_lookup_read_data(68),
      DOBDO(12) => mod_lookup_read_data(67),
      DOBDO(11) => mod_lookup_read_data(66),
      DOBDO(10) => mod_lookup_read_data(65),
      DOBDO(9) => mod_lookup_read_data(64),
      DOBDO(8) => mod_lookup_read_data(63),
      DOBDO(7) => mod_lookup_read_data(61),
      DOBDO(6) => mod_lookup_read_data(60),
      DOBDO(5) => mod_lookup_read_data(59),
      DOBDO(4) => mod_lookup_read_data(58),
      DOBDO(3) => mod_lookup_read_data(57),
      DOBDO(2) => mod_lookup_read_data(56),
      DOBDO(1) => mod_lookup_read_data(55),
      DOBDO(0) => mod_lookup_read_data(54)
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTAINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y15",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCEINV : 
X_BUF
    generic map(
      LOC => "RAMB8_X0Y15",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWRENINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y15",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEAINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y15",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDENINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y15",
      PATHPULSE => 115 ps
    )
    port map (
      I => '1',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRSTINV : X_BUF
    generic map(
      LOC => "RAMB8_X0Y15",
      PATHPULSE => 115 ps
    )
    port map (
      I => '0',
      O => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT
    );
  ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram : X_RAMB8BWER
    generic map(
      DATA_WIDTH_A => 36,
      DATA_WIDTH_B => 36,
      DOA_REG => 0,
      DOB_REG => 0,
      EN_RSTRAM_A => FALSE,
      EN_RSTRAM_B => FALSE,
      RAM_MODE => "SDP",
      RST_PRIORITY_A => "CE",
      RST_PRIORITY_B => "CE",
      RSTTYPE => "SYNC",
      WRITE_MODE_A => "WRITE_FIRST",
      WRITE_MODE_B => "WRITE_FIRST",
      INIT_FILE => "NONE",
      SIM_COLLISION_CHECK => "ALL",
      INITP_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_08 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_09 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_10 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_11 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_12 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_13 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_14 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_15 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_16 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_17 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_18 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_19 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_A => X"00000",
      INIT_B => X"00000",
      SRVAL_A => X"00000",
      SRVAL_B => X"00000",
      LOC => "RAMB8_X0Y15"
    )
    port map (
      RSTBRST => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTBRST_INT,
      ENBRDEN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENBRDEN_INT,
      REGCEA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEA_INT,
      ENAWREN => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_ENAWREN_INT,
      CLKAWRCLK => clk_pll,
      CLKBRDCLK => clk_pll,
      REGCEBREGCE => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_REGCEBREGCE_INT,
      RSTA => 
ModulationMem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_WIDE_PRIM9_ram_RSTA_INT,
      WEAWEL(1) => SPI_mod_table_write_5972,
      WEAWEL(0) => SPI_mod_table_write_5972,
      WEBWEU(1) => SPI_mod_table_write_5972,
      WEBWEU(0) => SPI_mod_table_write_5972,
      ADDRAWRADDR(12) => SPI_MOD_LOOKUP_INDEX_7_0,
      ADDRAWRADDR(11) => SPI_MOD_LOOKUP_INDEX_6_0,
      ADDRAWRADDR(10) => SPI_MOD_LOOKUP_INDEX_5_0,
      ADDRAWRADDR(9) => SPI_MOD_LOOKUP_INDEX_4_0,
      ADDRAWRADDR(8) => SPI_MOD_LOOKUP_INDEX(3),
      ADDRAWRADDR(7) => SPI_MOD_LOOKUP_INDEX(2),
      ADDRAWRADDR(6) => SPI_MOD_LOOKUP_INDEX(1),
      ADDRAWRADDR(5) => SPI_MOD_LOOKUP_INDEX(0),
      ADDRAWRADDR(4) => GND,
      ADDRAWRADDR(3) => GND,
      ADDRAWRADDR(2) => GND,
      ADDRAWRADDR(1) => GND,
      ADDRAWRADDR(0) => GND,
      DIPBDIP(1) => SPI_MOD_LOOKUP_DATA(35),
      DIPBDIP(0) => SPI_MOD_LOOKUP_DATA(26),
      DIBDI(15) => SPI_MOD_LOOKUP_DATA(34),
      DIBDI(14) => SPI_MOD_LOOKUP_DATA(33),
      DIBDI(13) => SPI_MOD_LOOKUP_DATA(32),
      DIBDI(12) => SPI_MOD_LOOKUP_DATA(31),
      DIBDI(11) => SPI_MOD_LOOKUP_DATA(30),
      DIBDI(10) => SPI_MOD_LOOKUP_DATA(29),
      DIBDI(9) => SPI_MOD_LOOKUP_DATA(28),
      DIBDI(8) => SPI_MOD_LOOKUP_DATA(27),
      DIBDI(7) => SPI_MOD_LOOKUP_DATA(25),
      DIBDI(6) => SPI_MOD_LOOKUP_DATA(24),
      DIBDI(5) => SPI_MOD_LOOKUP_DATA(23),
      DIBDI(4) => SPI_MOD_LOOKUP_DATA(22),
      DIBDI(3) => SPI_MOD_LOOKUP_DATA(21),
      DIBDI(2) => SPI_MOD_LOOKUP_DATA(20),
      DIBDI(1) => SPI_MOD_LOOKUP_DATA(19),
      DIBDI(0) => SPI_MOD_LOOKUP_DATA(18),
      DIADI(15) => SPI_MOD_LOOKUP_DATA(16),
      DIADI(14) => SPI_MOD_LOOKUP_DATA(15),
      DIADI(13) => SPI_MOD_LOOKUP_DATA(14),
      DIADI(12) => SPI_MOD_LOOKUP_DATA(13),
      DIADI(11) => SPI_MOD_LOOKUP_DATA(12),
      DIADI(10) => SPI_MOD_LOOKUP_DATA(11),
      DIADI(9) => SPI_MOD_LOOKUP_DATA(10),
      DIADI(8) => SPI_MOD_LOOKUP_DATA(9),
      DIADI(7) => SPI_MOD_LOOKUP_DATA(7),
      DIADI(6) => SPI_MOD_LOOKUP_DATA(6),
      DIADI(5) => SPI_MOD_LOOKUP_DATA(5),
      DIADI(4) => SPI_MOD_LOOKUP_DATA(4),
      DIADI(3) => SPI_MOD_LOOKUP_DATA(3),
      DIADI(2) => SPI_MOD_LOOKUP_DATA(2),
      DIADI(1) => SPI_MOD_LOOKUP_DATA(1),
      DIADI(0) => SPI_MOD_LOOKUP_DATA(0),
      ADDRBRDADDR(12) => Modulation_OUTPUT_SAMPLE_7_0,
      ADDRBRDADDR(11) => Modulation_OUTPUT_SAMPLE_6_0,
      ADDRBRDADDR(10) => Modulation_OUTPUT_SAMPLE_5_0,
      ADDRBRDADDR(9) => Modulation_OUTPUT_SAMPLE_4_0,
      ADDRBRDADDR(8) => Modulation_OUTPUT_SAMPLE(3),
      ADDRBRDADDR(7) => Modulation_OUTPUT_SAMPLE(2),
      ADDRBRDADDR(6) => Modulation_OUTPUT_SAMPLE(1),
      ADDRBRDADDR(5) => Modulation_OUTPUT_SAMPLE(0),
      ADDRBRDADDR(4) => GND,
      ADDRBRDADDR(3) => GND,
      ADDRBRDADDR(2) => GND,
      ADDRBRDADDR(1) => GND,
      ADDRBRDADDR(0) => GND,
      DIPADIP(1) => SPI_MOD_LOOKUP_DATA(17),
      DIPADIP(0) => SPI_MOD_LOOKUP_DATA(8),
      DOADO(15) => mod_lookup_read_data(16),
      DOADO(14) => mod_lookup_read_data(15),
      DOADO(13) => mod_lookup_read_data(14),
      DOADO(12) => mod_lookup_read_data(13),
      DOADO(11) => mod_lookup_read_data(12),
      DOADO(10) => mod_lookup_read_data(11),
      DOADO(9) => mod_lookup_read_data(10),
      DOADO(8) => mod_lookup_read_data(9),
      DOADO(7) => mod_lookup_read_data(7),
      DOADO(6) => mod_lookup_read_data(6),
      DOADO(5) => mod_lookup_read_data(5),
      DOADO(4) => mod_lookup_read_data(4),
      DOADO(3) => mod_lookup_read_data(3),
      DOADO(2) => mod_lookup_read_data(2),
      DOADO(1) => mod_lookup_read_data(1),
      DOADO(0) => mod_lookup_read_data(0),
      DOPADOP(1) => mod_lookup_read_data(17),
      DOPADOP(0) => mod_lookup_read_data(8),
      DOPBDOP(1) => mod_lookup_read_data(35),
      DOPBDOP(0) => mod_lookup_read_data(26),
      DOBDO(15) => mod_lookup_read_data(34),
      DOBDO(14) => mod_lookup_read_data(33),
      DOBDO(13) => mod_lookup_read_data(32),
      DOBDO(12) => mod_lookup_read_data(31),
      DOBDO(11) => mod_lookup_read_data(30),
      DOBDO(10) => mod_lookup_read_data(29),
      DOBDO(9) => mod_lookup_read_data(28),
      DOBDO(8) => mod_lookup_read_data(27),
      DOBDO(7) => mod_lookup_read_data(25),
      DOBDO(6) => mod_lookup_read_data(24),
      DOBDO(5) => mod_lookup_read_data(23),
      DOBDO(4) => mod_lookup_read_data(22),
      DOBDO(3) => mod_lookup_read_data(21),
      DOBDO(2) => mod_lookup_read_data(20),
      DOBDO(1) => mod_lookup_read_data(19),
      DOBDO(0) => mod_lookup_read_data(18)
    );
  Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram : X_RAMB16BWER
    generic map(
      DATA_WIDTH_A => 9,
      DATA_WIDTH_B => 9,
      DOA_REG => 0,
      DOB_REG => 0,
      EN_RSTRAM_A => FALSE,
      EN_RSTRAM_B => FALSE,
      RST_PRIORITY_A => "CE",
      RST_PRIORITY_B => "CE",
      RSTTYPE => "SYNC",
      WRITE_MODE_A => "WRITE_FIRST",
      WRITE_MODE_B => "WRITE_FIRST",
      INITP_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INITP_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_00 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_01 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_02 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_03 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_04 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_05 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_06 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_07 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_08 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_09 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_0F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_10 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_11 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_12 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_13 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_14 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_15 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_16 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_17 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_18 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_19 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_1F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_20 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_21 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_22 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_23 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_24 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_25 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_26 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_27 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_28 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_29 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_2A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_2B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_2C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_2D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_2E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_2F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_30 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_31 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_32 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_33 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_34 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_35 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_36 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_37 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_38 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_39 => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_3A => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_3B => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_3C => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_3D => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_3E => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_3F => X"0000000000000000000000000000000000000000000000000000000000000000",
      INIT_A => X"000000000",
      INIT_B => X"000000000",
      SRVAL_A => X"000000000",
      SRVAL_B => X"000000000",
      SIM_COLLISION_CHECK => "ALL",
      SIM_DEVICE => "SPARTAN6",
      INIT_FILE => "NONE",
      LOC => "RAMB16_X1Y14"
    )
    port map (
      CLKA => clk_pll,
      CLKB => clk_pll,
      ENA => '1',
      ENB => '1',
      REGCEA => '0',
      REGCEB => '0',
      RSTA => '0',
      RSTB => '0',
      ADDRA(13) => Modulation_write_pos(10),
      ADDRA(12) => Modulation_write_pos(9),
      ADDRA(11) => Modulation_write_pos(8),
      ADDRA(10) => Modulation_write_pos(7),
      ADDRA(9) => Modulation_write_pos(6),
      ADDRA(8) => Modulation_write_pos(5),
      ADDRA(7) => Modulation_write_pos(4),
      ADDRA(6) => Modulation_write_pos(3),
      ADDRA(5) => Modulation_write_pos(2),
      ADDRA(4) => Modulation_write_pos(1),
      ADDRA(3) => Modulation_write_pos(0),
      ADDRA(2) => '0',
      ADDRA(1) => '0',
      ADDRA(0) => '0',
      ADDRB(13) => Modulation_read_pos(10),
      ADDRB(12) => Modulation_read_pos(9),
      ADDRB(11) => Modulation_read_pos(8),
      ADDRB(10) => Modulation_read_pos(7),
      ADDRB(9) => Modulation_read_pos(6),
      ADDRB(8) => Modulation_read_pos(5),
      ADDRB(7) => Modulation_read_pos(4),
      ADDRB(6) => Modulation_read_pos(3),
      ADDRB(5) => Modulation_read_pos(2),
      ADDRB(4) => Modulation_read_pos(1),
      ADDRB(3) => Modulation_read_pos(0),
      ADDRB(2) => '0',
      ADDRB(1) => '0',
      ADDRB(0) => '0',
      DIA(31) => '0',
      DIA(30) => '0',
      DIA(29) => '0',
      DIA(28) => '0',
      DIA(27) => '0',
      DIA(26) => '0',
      DIA(25) => '0',
      DIA(24) => '0',
      DIA(23) => '0',
      DIA(22) => '0',
      DIA(21) => '0',
      DIA(20) => '0',
      DIA(19) => '0',
      DIA(18) => '0',
      DIA(17) => '0',
      DIA(16) => '0',
      DIA(15) => '0',
      DIA(14) => '0',
      DIA(13) => '0',
      DIA(12) => '0',
      DIA(11) => '0',
      DIA(10) => '0',
      DIA(9) => '0',
      DIA(8) => '0',
      DIA(7) => SPI_MOD_FIFO_DATA(7),
      DIA(6) => SPI_MOD_FIFO_DATA(6),
      DIA(5) => SPI_MOD_FIFO_DATA(5),
      DIA(4) => SPI_MOD_FIFO_DATA(4),
      DIA(3) => SPI_MOD_FIFO_DATA(3),
      DIA(2) => SPI_MOD_FIFO_DATA(2),
      DIA(1) => SPI_MOD_FIFO_DATA(1),
      DIA(0) => SPI_MOD_FIFO_DATA(0),
      DIB(31) => '0',
      DIB(30) => '0',
      DIB(29) => '0',
      DIB(28) => '0',
      DIB(27) => '0',
      DIB(26) => '0',
      DIB(25) => '0',
      DIB(24) => '0',
      DIB(23) => '0',
      DIB(22) => '0',
      DIB(21) => '0',
      DIB(20) => '0',
      DIB(19) => '0',
      DIB(18) => '0',
      DIB(17) => '0',
      DIB(16) => '0',
      DIB(15) => '0',
      DIB(14) => '0',
      DIB(13) => '0',
      DIB(12) => '0',
      DIB(11) => '0',
      DIB(10) => '0',
      DIB(9) => '0',
      DIB(8) => '0',
      DIB(7) => '0',
      DIB(6) => '0',
      DIB(5) => '0',
      DIB(4) => '0',
      DIB(3) => '0',
      DIB(2) => '0',
      DIB(1) => '0',
      DIB(0) => '0',
      DIPA(3) => '0',
      DIPA(2) => '0',
      DIPA(1) => '0',
      DIPA(0) => '0',
      DIPB(3) => '0',
      DIPB(2) => '0',
      DIPB(1) => '0',
      DIPB(0) => '0',
      DOA(31) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_31_UNCONNECTED
,
      DOA(30) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_30_UNCONNECTED
,
      DOA(29) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_29_UNCONNECTED
,
      DOA(28) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_28_UNCONNECTED
,
      DOA(27) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_27_UNCONNECTED
,
      DOA(26) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_26_UNCONNECTED
,
      DOA(25) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_25_UNCONNECTED
,
      DOA(24) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_24_UNCONNECTED
,
      DOA(23) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_23_UNCONNECTED
,
      DOA(22) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_22_UNCONNECTED
,
      DOA(21) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_21_UNCONNECTED
,
      DOA(20) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_20_UNCONNECTED
,
      DOA(19) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_19_UNCONNECTED
,
      DOA(18) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_18_UNCONNECTED
,
      DOA(17) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_17_UNCONNECTED
,
      DOA(16) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_16_UNCONNECTED
,
      DOA(15) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_15_UNCONNECTED
,
      DOA(14) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_14_UNCONNECTED
,
      DOA(13) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_13_UNCONNECTED
,
      DOA(12) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_12_UNCONNECTED
,
      DOA(11) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_11_UNCONNECTED
,
      DOA(10) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_10_UNCONNECTED
,
      DOA(9) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_9_UNCONNECTED
,
      DOA(8) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_8_UNCONNECTED
,
      DOA(7) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_7_UNCONNECTED
,
      DOA(6) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_6_UNCONNECTED
,
      DOA(5) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_5_UNCONNECTED
,
      DOA(4) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_4_UNCONNECTED
,
      DOA(3) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_3_UNCONNECTED
,
      DOA(2) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_2_UNCONNECTED
,
      DOA(1) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_1_UNCONNECTED
,
      DOA(0) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOA_0_UNCONNECTED
,
      DOB(31) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_31_UNCONNECTED
,
      DOB(30) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_30_UNCONNECTED
,
      DOB(29) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_29_UNCONNECTED
,
      DOB(28) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_28_UNCONNECTED
,
      DOB(27) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_27_UNCONNECTED
,
      DOB(26) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_26_UNCONNECTED
,
      DOB(25) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_25_UNCONNECTED
,
      DOB(24) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_24_UNCONNECTED
,
      DOB(23) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_23_UNCONNECTED
,
      DOB(22) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_22_UNCONNECTED
,
      DOB(21) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_21_UNCONNECTED
,
      DOB(20) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_20_UNCONNECTED
,
      DOB(19) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_19_UNCONNECTED
,
      DOB(18) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_18_UNCONNECTED
,
      DOB(17) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_17_UNCONNECTED
,
      DOB(16) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_16_UNCONNECTED
,
      DOB(15) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_15_UNCONNECTED
,
      DOB(14) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_14_UNCONNECTED
,
      DOB(13) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_13_UNCONNECTED
,
      DOB(12) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_12_UNCONNECTED
,
      DOB(11) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_11_UNCONNECTED
,
      DOB(10) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_10_UNCONNECTED
,
      DOB(9) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_9_UNCONNECTED
,
      DOB(8) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOB_8_UNCONNECTED
,
      DOB(7) => Modulation_sample(7),
      DOB(6) => Modulation_sample(6),
      DOB(5) => Modulation_sample(5),
      DOB(4) => Modulation_sample(4),
      DOB(3) => Modulation_sample(3),
      DOB(2) => Modulation_sample(2),
      DOB(1) => Modulation_sample(1),
      DOB(0) => Modulation_sample(0),
      DOPA(3) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_3_UNCONNECTED
,
      DOPA(2) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_2_UNCONNECTED
,
      DOPA(1) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_1_UNCONNECTED
,
      DOPA(0) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPA_0_UNCONNECTED
,
      DOPB(3) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_3_UNCONNECTED
,
      DOPB(2) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_2_UNCONNECTED
,
      DOPB(1) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_1_UNCONNECTED
,
      DOPB(0) => 
NLW_Modulation_Mem_U0_xst_blk_mem_generator_gnativebmg_native_blk_mem_gen_valid_cstr_ramloop_0_ram_r_s6_noinit_ram_SDP_SIMPLE_PRIM18_ram_DOPB_0_UNCONNECTED
,
      WEA(3) => mod_sample_latch,
      WEA(2) => mod_sample_latch,
      WEA(1) => mod_sample_latch,
      WEA(0) => mod_sample_latch,
      WEB(3) => '0',
      WEB(2) => '0',
      WEB(1) => '0',
      WEB(0) => '0'
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_3_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"C30000C3C30000C3"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(6),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_6_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(7),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_7_0,
      ADR5 => '1',
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_3_Q_555
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi3 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"30FF0030"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(6),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_6_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(7),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_7_0,
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi3_556
    );
  ProtoComp29_CYINITVCC : X_ONE
    generic map(
      LOC => "SLICE_X16Y20"
    )
    port map (
      O => ProtoComp29_CYINITVCC_1
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y20"
    )
    port map (
      CI => '0',
      CYINIT => ProtoComp29_CYINITVCC_1,
      CO(3) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_Q_6314,
      CO(2) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_CO_0_UNCONNECTED,
      DI(3) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi3_556,
      DI(2) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi2_564,
      DI(1) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi1_570,
      DI(0) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi_576,
      O(3) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_3_UNCONNECTED,
      O(2) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_2_UNCONNECTED,
      O(1) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_1_UNCONNECTED,
      O(0) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_O_0_UNCONNECTED,
      S(3) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_3_Q_555,
      S(2) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_2_Q_563,
      S(1) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_1_Q_569,
      S(0) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_0_Q_575
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_2_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"C30000C3C30000C3"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(4),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_4_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(5),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_5_0,
      ADR5 => '1',
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_2_Q_563
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi2 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"30FF0030"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(4),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_4_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(5),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_5_0,
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi2_564
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_1_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"C30000C3C30000C3"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(2),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_2_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(3),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_3_0,
      ADR5 => '1',
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_1_Q_569
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi1 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"30FF0030"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(2),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_2_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(3),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_3_0,
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi1_570
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_0_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"C30000C3C30000C3"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(0),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_0_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(1),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_1_0,
      ADR5 => '1',
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_0_Q_575
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi : X_LUT5
    generic map(
      LOC => "SLICE_X16Y20",
      INIT => X"30FF0030"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(0),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_0_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(1),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_1_0,
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi_576
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y21"
    )
    port map (
      CI => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_3_Q_6314,
      CYINIT => '0',
      CO(3) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_CO_3_UNCONNECTED,
      CO(2) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_CO_2_UNCONNECTED,
      CO(1) => Modulation_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o,
      CO(0) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_CO_0_UNCONNECTED,
      DI(3) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_DI_3_UNCONNECTED,
      DI(2) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_DI_2_UNCONNECTED,
      DI(1) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi5_591,
      DI(0) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi4_595,
      O(3) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_3_UNCONNECTED,
      O(2) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_2_UNCONNECTED,
      O(1) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_1_UNCONNECTED,
      O(0) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_O_0_UNCONNECTED,
      S(3) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_S_3_UNCONNECTED,
      S(2) => NLW_Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_cy_5_S_2_UNCONNECTED,
      S(1) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_5_Q_590,
      S(0) => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_4_Q_594
    );
  Modulation_THRESHOLD_CROSSED : X_SFF
    generic map(
      LOC => "SLICE_X16Y21",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o,
      O => Modulation_THRESHOLD_CROSSED_6320,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_5_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y21",
      INIT => X"FF0000FFFF0000FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_FIFO_THRESHOLD(10),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_0,
      ADR5 => '1',
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_5_Q_590
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi5 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y21",
      INIT => X"00FF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_FIFO_THRESHOLD(10),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_0,
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi5_591
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_4_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y21",
      INIT => X"C30000C3C30000C3"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(8),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_8_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(9),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_9_0,
      ADR5 => '1',
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lut_4_Q_594
    );
  Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi4 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y21",
      INIT => X"30FF0030"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_MOD_FIFO_THRESHOLD(8),
      ADR2 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_8_0,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(9),
      ADR4 => Modulation_GND_22_o_GND_22_o_sub_1_OUT_9_0,
      O => Modulation_Mcompar_THRESHOLD_LEVEL_10_GND_22_o_LessThan_2_o_lutdi4_595
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(3),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_3_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(2),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_2_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(1),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_1_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(0),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_0_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y22",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(3),
      ADR5 => Modulation_read_pos(3),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_1_603
    );
  ProtoComp31_CYINITVCC : X_ONE
    generic map(
      LOC => "SLICE_X16Y22"
    )
    port map (
      O => ProtoComp31_CYINITVCC_1
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y22"
    )
    port map (
      CI => '0',
      CYINIT => ProtoComp31_CYINITVCC_1,
      CO(3) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Q_6321,
      CO(2) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_CO_0_UNCONNECTED,
      DI(3) => Modulation_write_pos(3),
      DI(2) => Modulation_write_pos(2),
      DI(1) => Modulation_write_pos(1),
      DI(0) => Modulation_write_pos(0),
      O(3) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(3),
      O(2) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(2),
      O(1) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(1),
      O(0) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(0),
      S(3) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_1_603,
      S(2) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_2_Q_613,
      S(1) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_1_1_617,
      S(0) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_0_Q_621
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_2_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y22",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(2),
      ADR5 => Modulation_read_pos(2),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_2_Q_613
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_1_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y22",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(1),
      ADR5 => Modulation_read_pos(1),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_1_1_617
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_0_Q : X_LUT6
    generic map(
      LOC => "SLICE_X16Y22",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(0),
      ADR5 => Modulation_read_pos(0),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_0_Q_621
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(7),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_7_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(6),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_6_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(5),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_5_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(4),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_4_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_7_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y23",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(7),
      ADR5 => Modulation_read_pos(7),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_7_1_625
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y23"
    )
    port map (
      CI => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_3_Q_6321,
      CYINIT => '0',
      CO(3) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Q_6322,
      CO(2) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_CO_0_UNCONNECTED,
      DI(3) => Modulation_write_pos(7),
      DI(2) => Modulation_write_pos(6),
      DI(1) => Modulation_write_pos(5),
      DI(0) => Modulation_write_pos(4),
      O(3) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(7),
      O(2) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(6),
      O(1) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(5),
      O(0) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(4),
      S(3) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_7_1_625,
      S(2) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_1_635,
      S(1) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_1_639,
      S(0) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_1_643
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y23",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(6),
      ADR5 => Modulation_read_pos(6),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_1_635
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y23",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(5),
      ADR5 => Modulation_read_pos(5),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_1_639
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y23",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(4),
      ADR5 => Modulation_read_pos(4),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_1_643
    );
  Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(10),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_0
    );
  Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(9),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_9_0
    );
  Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_Modulation_GND_22_o_GND_22_o_sub_1_OUT_10_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_GND_22_o_GND_22_o_sub_1_OUT(8),
      O => Modulation_GND_22_o_GND_22_o_sub_1_OUT_8_0
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y24"
    )
    port map (
      CI => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_cy_7_Q_6322,
      CYINIT => '0',
      CO(3) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_3_UNCONNECTED,
      CO(2) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_CO_0_UNCONNECTED,
      DI(3) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_DI_3_UNCONNECTED,
      DI(2) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_DI_2_UNCONNECTED,
      DI(1) => Modulation_write_pos(9),
      DI(0) => Modulation_write_pos(8),
      O(3) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_O_3_UNCONNECTED,
      O(2) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(10),
      O(1) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(9),
      O(0) => Modulation_GND_22_o_GND_22_o_sub_1_OUT(8),
      S(3) => NLW_Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_xor_10_S_3_UNCONNECTED,
      S(2) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_1_650,
      S(1) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_1_654,
      S(0) => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_8_1_658
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y24",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(10),
      ADR5 => Modulation_read_pos(10),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_1_650
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y24",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(9),
      ADR5 => Modulation_read_pos(9),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_1_654
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_8_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y24",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(8),
      ADR5 => Modulation_read_pos(8),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_8_1_658
    );
  Modulation_Mcount_read_pos_cy_3_Modulation_Mcount_read_pos_cy_3_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_3_1,
      O => Modulation_Result_3_1_0
    );
  Modulation_Mcount_read_pos_cy_3_Modulation_Mcount_read_pos_cy_3_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_2_1,
      O => Modulation_Result_2_1_0
    );
  Modulation_Mcount_read_pos_cy_3_Modulation_Mcount_read_pos_cy_3_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_1_1,
      O => Modulation_Result_1_1_0
    );
  Modulation_Mcount_read_pos_cy_3_Modulation_Mcount_read_pos_cy_3_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_0_1,
      O => Modulation_Result_0_1_0
    );
  Modulation_read_pos_3_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(3),
      ADR5 => '1',
      O => Modulation_read_pos_3_rt_661
    );
  REF_SCLK_OBUF_1_7_D5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_7_D5LUT_O_UNCONNECTED
    );
  ProtoComp34_CYINITGND : X_ZERO
    generic map(
      LOC => "SLICE_X18Y26"
    )
    port map (
      O => ProtoComp34_CYINITGND_0
    );
  Modulation_Mcount_read_pos_cy_3_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X18Y26"
    )
    port map (
      CI => '0',
      CYINIT => ProtoComp34_CYINITGND_0,
      CO(3) => Modulation_Mcount_read_pos_cy_3_Q_6326,
      CO(2) => NLW_Modulation_Mcount_read_pos_cy_3_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Mcount_read_pos_cy_3_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Mcount_read_pos_cy_3_CO_0_UNCONNECTED,
      DI(3) => '0',
      DI(2) => '0',
      DI(1) => '0',
      DI(0) => '1',
      O(3) => Modulation_Result_3_1,
      O(2) => Modulation_Result_2_1,
      O(1) => Modulation_Result_1_1,
      O(0) => Modulation_Result_0_1,
      S(3) => Modulation_read_pos_3_rt_661,
      S(2) => Modulation_read_pos_2_rt_670,
      S(1) => Modulation_read_pos_1_rt_673,
      S(0) => Modulation_Mcount_read_pos_lut(0)
    );
  Modulation_read_pos_2_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(2),
      ADR5 => '1',
      O => Modulation_read_pos_2_rt_670
    );
  REF_SCLK_OBUF_1_8_C5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_8_C5LUT_O_UNCONNECTED
    );
  Modulation_read_pos_1_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(1),
      ADR5 => '1',
      O => Modulation_read_pos_1_rt_673
    );
  REF_SCLK_OBUF_1_9_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_9_B5LUT_O_UNCONNECTED
    );
  Modulation_Mcount_read_pos_lut_0_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"0000FFFF0000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(0),
      ADR5 => '1',
      O => Modulation_Mcount_read_pos_lut(0)
    );
  REF_MIX1_EN_OBUF_1_2_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y26",
      INIT => X"FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_MIX1_EN_OBUF_1_2_A5LUT_O_UNCONNECTED
    );
  Modulation_Mcount_read_pos_cy_7_Modulation_Mcount_read_pos_cy_7_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_7_1,
      O => Modulation_Result_7_1_0
    );
  Modulation_Mcount_read_pos_cy_7_Modulation_Mcount_read_pos_cy_7_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_6_1,
      O => Modulation_Result_6_1_0
    );
  Modulation_Mcount_read_pos_cy_7_Modulation_Mcount_read_pos_cy_7_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_5_1,
      O => Modulation_Result_5_1_0
    );
  Modulation_Mcount_read_pos_cy_7_Modulation_Mcount_read_pos_cy_7_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_4_1,
      O => Modulation_Result_4_1_0
    );
  Modulation_read_pos_7_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(7),
      ADR5 => '1',
      O => Modulation_read_pos_7_rt_683
    );
  REF_SCLK_OBUF_1_3_D5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_3_D5LUT_O_UNCONNECTED
    );
  Modulation_Mcount_read_pos_cy_7_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X18Y27"
    )
    port map (
      CI => Modulation_Mcount_read_pos_cy_3_Q_6326,
      CYINIT => '0',
      CO(3) => Modulation_Mcount_read_pos_cy_7_Q_6331,
      CO(2) => NLW_Modulation_Mcount_read_pos_cy_7_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Mcount_read_pos_cy_7_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Mcount_read_pos_cy_7_CO_0_UNCONNECTED,
      DI(3) => '0',
      DI(2) => '0',
      DI(1) => '0',
      DI(0) => '0',
      O(3) => Modulation_Result_7_1,
      O(2) => Modulation_Result_6_1,
      O(1) => Modulation_Result_5_1,
      O(0) => Modulation_Result_4_1,
      S(3) => Modulation_read_pos_7_rt_683,
      S(2) => Modulation_read_pos_6_rt_692,
      S(1) => Modulation_read_pos_5_rt_695,
      S(0) => Modulation_read_pos_4_rt_698
    );
  Modulation_read_pos_6_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(6),
      ADR5 => '1',
      O => Modulation_read_pos_6_rt_692
    );
  REF_SCLK_OBUF_1_4_C5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_4_C5LUT_O_UNCONNECTED
    );
  Modulation_read_pos_5_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(5),
      ADR5 => '1',
      O => Modulation_read_pos_5_rt_695
    );
  REF_SCLK_OBUF_1_5_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_5_B5LUT_O_UNCONNECTED
    );
  Modulation_read_pos_4_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(4),
      ADR5 => '1',
      O => Modulation_read_pos_4_rt_698
    );
  REF_SCLK_OBUF_1_6_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y27",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_6_A5LUT_O_UNCONNECTED
    );
  Modulation_Result_10_1_Modulation_Result_10_1_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_10_1,
      O => Modulation_Result_10_1_0
    );
  Modulation_Result_10_1_Modulation_Result_10_1_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_9_1,
      O => Modulation_Result_9_1_0
    );
  Modulation_Result_10_1_Modulation_Result_10_1_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_Result_8_1,
      O => Modulation_Result_8_1_0
    );
  Modulation_Mcount_read_pos_xor_10_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X18Y28"
    )
    port map (
      CI => Modulation_Mcount_read_pos_cy_7_Q_6331,
      CYINIT => '0',
      CO(3) => NLW_Modulation_Mcount_read_pos_xor_10_CO_3_UNCONNECTED,
      CO(2) => NLW_Modulation_Mcount_read_pos_xor_10_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Mcount_read_pos_xor_10_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Mcount_read_pos_xor_10_CO_0_UNCONNECTED,
      DI(3) => NLW_Modulation_Mcount_read_pos_xor_10_DI_3_UNCONNECTED,
      DI(2) => NLW_Modulation_Mcount_read_pos_xor_10_DI_2_UNCONNECTED,
      DI(1) => '0',
      DI(0) => '0',
      O(3) => NLW_Modulation_Mcount_read_pos_xor_10_O_3_UNCONNECTED,
      O(2) => Modulation_Result_10_1,
      O(1) => Modulation_Result_9_1,
      O(0) => Modulation_Result_8_1,
      S(3) => NLW_Modulation_Mcount_read_pos_xor_10_S_3_UNCONNECTED,
      S(2) => Modulation_read_pos_10_rt_709,
      S(1) => Modulation_read_pos_9_rt_711,
      S(0) => Modulation_read_pos_8_rt_714
    );
  Modulation_read_pos_10_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y28",
      INIT => X"FFFFFFFF00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Modulation_read_pos(10),
      O => Modulation_read_pos_10_rt_709
    );
  Modulation_read_pos_9_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y28",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(9),
      ADR5 => '1',
      O => Modulation_read_pos_9_rt_711
    );
  REF_SCLK_OBUF_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y28",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_B5LUT_O_UNCONNECTED
    );
  Modulation_read_pos_8_rt : X_LUT6
    generic map(
      LOC => "SLICE_X18Y28",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_read_pos(8),
      ADR5 => '1',
      O => Modulation_read_pos_8_rt_714
    );
  REF_SCLK_OBUF_1_2_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X18Y28",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_2_A5LUT_O_UNCONNECTED
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_3_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_3_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_2_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_2_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_1_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_1_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_0_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_0_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_3_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y9",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(3),
      ADR5 => SPI_MOD_PHASE_INC(3),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_3_Q_720
    );
  ProtoComp37_CYINITGND : X_ZERO
    generic map(
      LOC => "SLICE_X6Y9"
    )
    port map (
      O => ProtoComp37_CYINITGND_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X6Y9"
    )
    port map (
      CI => '0',
      CYINIT => ProtoComp37_CYINITGND_0,
      CO(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Q_6339,
      CO(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_CO_0_UNCONNECTED,
      DI(3) => Modulation_clk_sample_cnt(3),
      DI(2) => Modulation_clk_sample_cnt(2),
      DI(1) => Modulation_clk_sample_cnt(1),
      DI(0) => Modulation_clk_sample_cnt(0),
      O(3) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_3_Q,
      O(2) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_2_Q,
      O(1) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_1_Q,
      O(0) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_0_Q,
      S(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_3_Q_720,
      S(2) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_2_Q_730,
      S(1) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_1_Q_734,
      S(0) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_0_Q_738
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_2_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y9",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(2),
      ADR5 => SPI_MOD_PHASE_INC(2),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_2_Q_730
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_1_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y9",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(1),
      ADR5 => SPI_MOD_PHASE_INC(1),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_1_Q_734
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_0_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y9",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(0),
      ADR5 => SPI_MOD_PHASE_INC(0),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_0_Q_738
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_7_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_7_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_6_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_6_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_5_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_5_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_4_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_4_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_7_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y10",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(7),
      ADR5 => SPI_MOD_PHASE_INC(7),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_7_Q_742
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X6Y10"
    )
    port map (
      CI => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_3_Q_6339,
      CYINIT => '0',
      CO(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Q_6344,
      CO(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_CO_0_UNCONNECTED,
      DI(3) => Modulation_clk_sample_cnt(7),
      DI(2) => Modulation_clk_sample_cnt(6),
      DI(1) => Modulation_clk_sample_cnt(5),
      DI(0) => Modulation_clk_sample_cnt(4),
      O(3) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_7_Q,
      O(2) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_6_Q,
      O(1) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_5_Q,
      O(0) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_4_Q,
      S(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_7_Q_742,
      S(2) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_6_Q_752,
      S(1) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_5_Q_756,
      S(0) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_4_Q_760
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_6_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y10",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(6),
      ADR5 => SPI_MOD_PHASE_INC(6),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_6_Q_752
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_5_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y10",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(5),
      ADR5 => SPI_MOD_PHASE_INC(5),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_5_Q_756
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_4_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y10",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(4),
      ADR5 => SPI_MOD_PHASE_INC(4),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_4_Q_760
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_11_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_11_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_10_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_10_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_9_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_9_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_8_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_8_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_11_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y11",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(11),
      ADR5 => SPI_MOD_PHASE_INC(11),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_11_Q_764
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X6Y11"
    )
    port map (
      CI => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_7_Q_6344,
      CYINIT => '0',
      CO(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Q_6349,
      CO(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_CO_0_UNCONNECTED,
      DI(3) => Modulation_clk_sample_cnt(11),
      DI(2) => Modulation_clk_sample_cnt(10),
      DI(1) => Modulation_clk_sample_cnt(9),
      DI(0) => Modulation_clk_sample_cnt(8),
      O(3) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_11_Q,
      O(2) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_10_Q,
      O(1) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_9_Q,
      O(0) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_8_Q,
      S(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_11_Q_764,
      S(2) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_10_Q_774,
      S(1) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_9_Q_778,
      S(0) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_8_Q_782
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_10_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y11",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(10),
      ADR5 => SPI_MOD_PHASE_INC(10),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_10_Q_774
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_9_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y11",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(9),
      ADR5 => SPI_MOD_PHASE_INC(9),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_9_Q_778
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_8_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y11",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(8),
      ADR5 => SPI_MOD_PHASE_INC(8),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_8_Q_782
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_15_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_15_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_14_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_14_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_13_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_13_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_12_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_12_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_15_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y12",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(15),
      ADR5 => SPI_MOD_PHASE_INC(15),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_15_Q_786
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X6Y12"
    )
    port map (
      CI => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_11_Q_6349,
      CYINIT => '0',
      CO(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Q_6354,
      CO(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_CO_0_UNCONNECTED,
      DI(3) => Modulation_clk_sample_cnt(15),
      DI(2) => Modulation_clk_sample_cnt(14),
      DI(1) => Modulation_clk_sample_cnt(13),
      DI(0) => Modulation_clk_sample_cnt(12),
      O(3) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_15_Q,
      O(2) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_14_Q,
      O(1) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_13_Q,
      O(0) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_12_Q,
      S(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_15_Q_786,
      S(2) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_14_Q_796,
      S(1) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_13_Q_800,
      S(0) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_12_Q_804
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_14_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y12",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(14),
      ADR5 => SPI_MOD_PHASE_INC(14),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_14_Q_796
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_13_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y12",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(13),
      ADR5 => SPI_MOD_PHASE_INC(13),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_13_Q_800
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_12_Q : X_LUT6
    generic map(
      LOC => "SLICE_X6Y12",
      INIT => X"0000FFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(12),
      ADR5 => SPI_MOD_PHASE_INC(12),
      O => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_lut_12_Q_804
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_19_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_19_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_18_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_18_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_17_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_17_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_16_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_16_0
    );
  Modulation_clk_sample_cnt_19_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(19),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_19_rt_807
    );
  REF_SCLK_OBUF_1_29_D5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_29_D5LUT_O_UNCONNECTED
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X6Y13"
    )
    port map (
      CI => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_15_Q_6354,
      CYINIT => '0',
      CO(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Q_6356,
      CO(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_CO_0_UNCONNECTED,
      DI(3) => '0',
      DI(2) => '0',
      DI(1) => '0',
      DI(0) => '0',
      O(3) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_19_Q,
      O(2) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_18_Q,
      O(1) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_17_Q,
      O(0) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_16_Q,
      S(3) => Modulation_clk_sample_cnt_19_rt_807,
      S(2) => Modulation_clk_sample_cnt_18_rt_816,
      S(1) => Modulation_clk_sample_cnt_17_rt_819,
      S(0) => Modulation_clk_sample_cnt_16_rt_822
    );
  Modulation_clk_sample_cnt_18_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(18),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_18_rt_816
    );
  REF_SCLK_OBUF_1_30_C5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_30_C5LUT_O_UNCONNECTED
    );
  Modulation_clk_sample_cnt_17_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(17),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_17_rt_819
    );
  REF_SCLK_OBUF_1_31_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_31_B5LUT_O_UNCONNECTED
    );
  Modulation_clk_sample_cnt_16_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(16),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_16_rt_822
    );
  REF_SCLK_OBUF_1_32_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y13",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_32_A5LUT_O_UNCONNECTED
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_23_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_23_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_22_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_22_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_21_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_21_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_20_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_20_0
    );
  Modulation_clk_sample_cnt_23_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(23),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_23_rt_829
    );
  REF_SCLK_OBUF_1_25_D5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_25_D5LUT_O_UNCONNECTED
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X6Y14"
    )
    port map (
      CI => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_19_Q_6356,
      CYINIT => '0',
      CO(3) => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Q_6357,
      CO(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_CO_0_UNCONNECTED,
      DI(3) => '0',
      DI(2) => '0',
      DI(1) => '0',
      DI(0) => '0',
      O(3) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_23_Q,
      O(2) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_22_Q,
      O(1) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_21_Q,
      O(0) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_20_Q,
      S(3) => Modulation_clk_sample_cnt_23_rt_829,
      S(2) => Modulation_clk_sample_cnt_22_rt_838,
      S(1) => Modulation_clk_sample_cnt_21_rt_841,
      S(0) => Modulation_clk_sample_cnt_20_rt_844
    );
  Modulation_clk_sample_cnt_22_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(22),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_22_rt_838
    );
  REF_SCLK_OBUF_1_26_C5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_26_C5LUT_O_UNCONNECTED
    );
  Modulation_clk_sample_cnt_21_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(21),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_21_rt_841
    );
  REF_SCLK_OBUF_1_27_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_27_B5LUT_O_UNCONNECTED
    );
  Modulation_clk_sample_cnt_20_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(20),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_20_rt_844
    );
  REF_SCLK_OBUF_1_28_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y14",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_28_A5LUT_O_UNCONNECTED
    );
  Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_0
    );
  Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_25_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_25_0
    );
  Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_24_Q,
      O => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_24_0
    );
  Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X6Y15"
    )
    port map (
      CI => Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_cy_23_Q_6357,
      CYINIT => '0',
      CO(3) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_3_UNCONNECTED,
      CO(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_CO_0_UNCONNECTED,
      DI(3) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_DI_3_UNCONNECTED,
      DI(2) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_DI_2_UNCONNECTED,
      DI(1) => '0',
      DI(0) => '0',
      O(3) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_O_3_UNCONNECTED,
      O(2) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_Q,
      O(1) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_25_Q,
      O(0) => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_24_Q,
      S(3) => NLW_Modulation_Madd_clk_sample_cnt_26_GND_22_o_add_5_OUT_xor_26_S_3_UNCONNECTED,
      S(2) => Modulation_clk_sample_cnt_26_rt_855,
      S(1) => Modulation_clk_sample_cnt_25_rt_857,
      S(0) => Modulation_clk_sample_cnt_24_rt_860
    );
  Modulation_clk_sample_cnt_26_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y15",
      INIT => X"FFFFFFFF00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Modulation_clk_sample_cnt(26),
      O => Modulation_clk_sample_cnt_26_rt_855
    );
  Modulation_clk_sample_cnt_25_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y15",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(25),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_25_rt_857
    );
  REF_SCLK_OBUF_1_23_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y15",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_23_B5LUT_O_UNCONNECTED
    );
  Modulation_clk_sample_cnt_24_rt : X_LUT6
    generic map(
      LOC => "SLICE_X6Y15",
      INIT => X"FFFF0000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_clk_sample_cnt(24),
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_24_rt_860
    );
  REF_SCLK_OBUF_1_24_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X6Y15",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_24_A5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_3 : X_SFF
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(3),
      O => Modulation_write_pos(3),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_write_pos_3_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(3),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_3_rt_867
    );
  REF_SCLK_OBUF_1_18_D5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_18_D5LUT_O_UNCONNECTED
    );
  ProtoComp39_CYINITGND : X_ZERO
    generic map(
      LOC => "SLICE_X16Y25"
    )
    port map (
      O => ProtoComp39_CYINITGND_0
    );
  Modulation_write_pos_2 : X_SFF
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(2),
      O => Modulation_write_pos(2),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_Mcount_write_pos_cy_3_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y25"
    )
    port map (
      CI => '0',
      CYINIT => ProtoComp39_CYINITGND_0,
      CO(3) => Modulation_Mcount_write_pos_cy_3_Q_6359,
      CO(2) => NLW_Modulation_Mcount_write_pos_cy_3_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Mcount_write_pos_cy_3_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Mcount_write_pos_cy_3_CO_0_UNCONNECTED,
      DI(3) => '0',
      DI(2) => '0',
      DI(1) => '0',
      DI(0) => '1',
      O(3) => Modulation_Result(3),
      O(2) => Modulation_Result(2),
      O(1) => Modulation_Result(1),
      O(0) => Modulation_Result(0),
      S(3) => Modulation_write_pos_3_rt_867,
      S(2) => Modulation_write_pos_2_rt_879,
      S(1) => Modulation_write_pos_1_rt_883,
      S(0) => Modulation_Mcount_write_pos_lut(0)
    );
  Modulation_write_pos_2_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(2),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_2_rt_879
    );
  REF_SCLK_OBUF_1_19_C5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_19_C5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_1 : X_SFF
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(1),
      O => Modulation_write_pos(1),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_write_pos_1_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(1),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_1_rt_883
    );
  REF_SCLK_OBUF_1_20_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_20_B5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_0 : X_SFF
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(0),
      O => Modulation_write_pos(0),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_Mcount_write_pos_lut_0_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"00FF00FF00FF00FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(0),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_Mcount_write_pos_lut(0)
    );
  REF_MIX1_EN_OBUF_1_3_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y25",
      INIT => X"FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_MIX1_EN_OBUF_1_3_A5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_7 : X_SFF
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(7),
      O => Modulation_write_pos(7),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_write_pos_7_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(7),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_7_rt_896
    );
  REF_SCLK_OBUF_1_14_D5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_14_D5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_6 : X_SFF
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(6),
      O => Modulation_write_pos(6),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_Mcount_write_pos_cy_7_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y26"
    )
    port map (
      CI => Modulation_Mcount_write_pos_cy_3_Q_6359,
      CYINIT => '0',
      CO(3) => Modulation_Mcount_write_pos_cy_7_Q_6360,
      CO(2) => NLW_Modulation_Mcount_write_pos_cy_7_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Mcount_write_pos_cy_7_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Mcount_write_pos_cy_7_CO_0_UNCONNECTED,
      DI(3) => '0',
      DI(2) => '0',
      DI(1) => '0',
      DI(0) => '0',
      O(3) => Modulation_Result(7),
      O(2) => Modulation_Result(6),
      O(1) => Modulation_Result(5),
      O(0) => Modulation_Result(4),
      S(3) => Modulation_write_pos_7_rt_896,
      S(2) => Modulation_write_pos_6_rt_908,
      S(1) => Modulation_write_pos_5_rt_912,
      S(0) => Modulation_write_pos_4_rt_916
    );
  Modulation_write_pos_6_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(6),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_6_rt_908
    );
  REF_SCLK_OBUF_1_15_C5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_15_C5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_5 : X_SFF
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(5),
      O => Modulation_write_pos(5),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_write_pos_5_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(5),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_5_rt_912
    );
  REF_SCLK_OBUF_1_16_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_16_B5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_4 : X_SFF
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(4),
      O => Modulation_write_pos(4),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_write_pos_4_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(4),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_4_rt_916
    );
  REF_SCLK_OBUF_1_17_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y26",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_17_A5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_10 : X_SFF
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(10),
      O => Modulation_write_pos(10),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_Mcount_write_pos_xor_10_Q : X_CARRY4
    generic map(
      LOC => "SLICE_X16Y27"
    )
    port map (
      CI => Modulation_Mcount_write_pos_cy_7_Q_6360,
      CYINIT => '0',
      CO(3) => NLW_Modulation_Mcount_write_pos_xor_10_CO_3_UNCONNECTED,
      CO(2) => NLW_Modulation_Mcount_write_pos_xor_10_CO_2_UNCONNECTED,
      CO(1) => NLW_Modulation_Mcount_write_pos_xor_10_CO_1_UNCONNECTED,
      CO(0) => NLW_Modulation_Mcount_write_pos_xor_10_CO_0_UNCONNECTED,
      DI(3) => NLW_Modulation_Mcount_write_pos_xor_10_DI_3_UNCONNECTED,
      DI(2) => NLW_Modulation_Mcount_write_pos_xor_10_DI_2_UNCONNECTED,
      DI(1) => '0',
      DI(0) => '0',
      O(3) => NLW_Modulation_Mcount_write_pos_xor_10_O_3_UNCONNECTED,
      O(2) => Modulation_Result(10),
      O(1) => Modulation_Result(9),
      O(0) => Modulation_Result(8),
      S(3) => NLW_Modulation_Mcount_write_pos_xor_10_S_3_UNCONNECTED,
      S(2) => Modulation_write_pos_10_rt_931,
      S(1) => Modulation_write_pos_9_rt_934,
      S(0) => Modulation_write_pos_8_rt_938
    );
  Modulation_write_pos_10_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(10),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_10_rt_931
    );
  Modulation_write_pos_9 : X_SFF
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(9),
      O => Modulation_write_pos(9),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_write_pos_9_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(9),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_9_rt_934
    );
  REF_SCLK_OBUF_1_12_B5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_12_B5LUT_O_UNCONNECTED
    );
  Modulation_write_pos_8 : X_SFF
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => '0'
    )
    port map (
      CE => mod_sample_latch,
      CLK => clk_pll,
      I => Modulation_Result(8),
      O => Modulation_write_pos(8),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_write_pos_8_rt : X_LUT6
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => X"FF00FF00FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_write_pos(8),
      ADR4 => '1',
      ADR5 => '1',
      O => Modulation_write_pos_8_rt_938
    );
  REF_SCLK_OBUF_1_13_A5LUT : X_LUT5
    generic map(
      LOC => "SLICE_X16Y27",
      INIT => X"00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => NLW_REF_SCLK_OBUF_1_13_A5LUT_O_UNCONNECTED
    );
  PORT1_CONVSTART_OBUF : X_OBUF
    generic map(
      LOC => "PAD6"
    )
    port map (
      I => '0',
      O => PORT1_CONVSTART
    );
  PORT1_MIX1_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD4"
    )
    port map (
      I => '1',
      O => PORT1_MIX1_EN
    );
  FILT_OUT_C1_OBUF : X_OBUF
    generic map(
      LOC => "PAD171"
    )
    port map (
      I => SPI_SOURCE_FILTER(0),
      O => FILT_OUT_C1
    );
  FILT_OUT_C2_OBUF : X_OBUF
    generic map(
      LOC => "PAD170"
    )
    port map (
      I => SPI_SOURCE_FILTER(1),
      O => FILT_OUT_C2
    );
  PORT1_MIX2_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD5"
    )
    port map (
      I => '0',
      O => PORT1_MIX2_EN
    );
  MainCLK_clkin1_buf : X_BUF
    generic map(
      LOC => "PAD122",
      PATHPULSE => 115 ps
    )
    port map (
      O => MainCLK_clkin1,
      I => CLK
    );
  ProtoComp43_IMUX : X_BUF
    generic map(
      LOC => "PAD122",
      PATHPULSE => 115 ps
    )
    port map (
      I => MainCLK_clkin1,
      O => MainCLK_clkin1_0
    );
  PORT1_SDO_IBUF : X_BUF
    generic map(
      LOC => "PAD7",
      PATHPULSE => 115 ps
    )
    port map (
      O => PORT1_SDO_IBUF_957,
      I => PORT1_SDO
    );
  ProtoComp43_IMUX_1 : X_BUF
    generic map(
      LOC => "PAD7",
      PATHPULSE => 115 ps
    )
    port map (
      I => PORT1_SDO_IBUF_957,
      O => PORT1_SDO_IBUF_0
    );
  PORT2_SDO_IBUF : X_BUF
    generic map(
      LOC => "PAD138",
      PATHPULSE => 115 ps
    )
    port map (
      O => PORT2_SDO_IBUF_960,
      I => PORT2_SDO
    );
  ProtoComp43_IMUX_2 : X_BUF
    generic map(
      LOC => "PAD138",
      PATHPULSE => 115 ps
    )
    port map (
      I => PORT2_SDO_IBUF_960,
      O => PORT2_SDO_IBUF_0
    );
  PORT2_MIX1_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD111"
    )
    port map (
      I => '1',
      O => PORT2_MIX1_EN
    );
  PORT2_MIX2_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD112"
    )
    port map (
      I => '0',
      O => PORT2_MIX2_EN
    );
  PORT1_SCLK_OBUF : X_OBUF
    generic map(
      LOC => "PAD8"
    )
    port map (
      I => '0',
      O => PORT1_SCLK
    );
  PORT2_SCLK_OBUF : X_OBUF
    generic map(
      LOC => "PAD141"
    )
    port map (
      I => '0',
      O => PORT2_SCLK
    );
  PORT2_CONVSTART_OBUF : X_OBUF
    generic map(
      LOC => "PAD137"
    )
    port map (
      I => '0',
      O => PORT2_CONVSTART
    );
  SOURCE_CE_OBUF : X_OBUF
    generic map(
      LOC => "PAD161"
    )
    port map (
      I => SPI_SOURCE_CE_EN_6366,
      O => SOURCE_CE
    );
  SOURCE_LD_IBUF : X_BUF
    generic map(
      LOC => "PAD145",
      PATHPULSE => 115 ps
    )
    port map (
      O => LEDS_2_OBUF_975,
      I => SOURCE_LD
    );
  ProtoComp43_IMUX_3 : X_BUF
    generic map(
      LOC => "PAD145",
      PATHPULSE => 115 ps
    )
    port map (
      I => LEDS_2_OBUF_975,
      O => LEDS_2_OBUF_0
    );
  SOURCE_LE_OBUF : X_OBUF
    generic map(
      LOC => "PAD160"
    )
    port map (
      I => SOURCE_LE_OBUF_5838,
      O => SOURCE_LE
    );
  RESET_IBUF : X_BUF
    generic map(
      LOC => "PAD83",
      PATHPULSE => 115 ps
    )
    port map (
      O => RESET_IBUF_980,
      I => RESET
    );
  ProtoComp43_IMUX_4 : X_BUF
    generic map(
      LOC => "PAD83",
      PATHPULSE => 115 ps
    )
    port map (
      I => RESET_IBUF_980,
      O => RESET_IBUF_0
    );
  SOURCE_CLK_OBUF : X_OBUF
    generic map(
      LOC => "PAD148"
    )
    port map (
      I => SOURCE_CLK_OBUF_0,
      O => SOURCE_CLK
    );
  SOURCE_MUX_IBUF : X_BUF
    generic map(
      LOC => "PAD147",
      PATHPULSE => 115 ps
    )
    port map (
      O => SOURCE_MUX_IBUF_985,
      I => SOURCE_MUX
    );
  ProtoComp43_IMUX_5 : X_BUF
    generic map(
      LOC => "PAD147",
      PATHPULSE => 115 ps
    )
    port map (
      I => SOURCE_MUX_IBUF_985,
      O => SOURCE_MUX_IBUF_0
    );
  SOURCE_RF_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD146"
    )
    port map (
      I => SPI_SOURCE_RF_EN_6369,
      O => SOURCE_RF_EN
    );
  REF_CONVSTART_OBUF : X_OBUF
    generic map(
      LOC => "PAD192"
    )
    port map (
      I => '0',
      O => REF_CONVSTART
    );
  LO1_CE_OBUF : X_OBUF
    generic map(
      LOC => "PAD136"
    )
    port map (
      I => '0',
      O => LO1_CE
    );
  LO1_LD_IBUF : X_BUF
    generic map(
      LOC => "PAD119",
      PATHPULSE => 115 ps
    )
    port map (
      O => LEDS_3_OBUF_994,
      I => LO1_LD
    );
  ProtoComp43_IMUX_6 : X_BUF
    generic map(
      LOC => "PAD119",
      PATHPULSE => 115 ps
    )
    port map (
      I => LEDS_3_OBUF_994,
      O => LEDS_3_OBUF_0
    );
  LO1_LE_OBUF : X_OBUF
    generic map(
      LOC => "PAD135"
    )
    port map (
      I => LO1_LE_OBUF_5957,
      O => LO1_LE
    );
  BAND_SELECT_HIGH_OBUF : X_OBUF
    generic map(
      LOC => "PAD172"
    )
    port map (
      I => BAND_SELECT_HIGH_OBUF_6371,
      O => BAND_SELECT_HIGH
    );
  FILT_IN_C1_OBUF : X_OBUF
    generic map(
      LOC => "PAD162"
    )
    port map (
      I => SPI_SOURCE_FILTER(0),
      O => FILT_IN_C1
    );
  FILT_IN_C2_OBUF : X_OBUF
    generic map(
      LOC => "PAD169"
    )
    port map (
      I => FILT_IN_C2_OBUF_6372,
      O => FILT_IN_C2
    );
  PORT1_SELECT_OBUF : X_OBUF
    generic map(
      LOC => "PAD3"
    )
    port map (
      I => PORT_SELECT1_OBUF_5828,
      O => PORT1_SELECT
    );
  PORT2_SELECT_OBUF : X_OBUF
    generic map(
      LOC => "PAD142"
    )
    port map (
      I => PORT_SELECT2_OBUF_0,
      O => PORT2_SELECT
    );
  SOURCE_MOSI_OBUF : X_OBUF
    generic map(
      LOC => "PAD159"
    )
    port map (
      I => SOURCE_MOSI_OBUF_6373,
      O => SOURCE_MOSI
    );
  BAND_SELECT_LOW_OBUF : X_OBUF
    generic map(
      LOC => "PAD173"
    )
    port map (
      I => SPI_SOURCE_BANDSELECT_6374,
      O => BAND_SELECT_LOW
    );
  LO1_RF_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD120"
    )
    port map (
      I => '0',
      O => LO1_RF_EN
    );
  ATTENUATION_0_OBUF : X_OBUF
    generic map(
      LOC => "PAD174"
    )
    port map (
      I => ATTENUATION_0_OBUF_6375,
      O => ATTENUATION(0)
    );
  ATTENUATION_1_OBUF : X_OBUF
    generic map(
      LOC => "PAD175"
    )
    port map (
      I => ATTENUATION_1_OBUF_6376,
      O => ATTENUATION(1)
    );
  ATTENUATION_2_OBUF : X_OBUF
    generic map(
      LOC => "PAD176"
    )
    port map (
      I => ATTENUATION_2_OBUF_6377,
      O => ATTENUATION(2)
    );
  ATTENUATION_3_OBUF : X_OBUF
    generic map(
      LOC => "PAD185"
    )
    port map (
      I => ATTENUATION_3_OBUF_6378,
      O => ATTENUATION(3)
    );
  ATTENUATION_4_OBUF : X_OBUF
    generic map(
      LOC => "PAD186"
    )
    port map (
      I => ATTENUATION_4_OBUF_6379,
      O => ATTENUATION(4)
    );
  ATTENUATION_5_OBUF : X_OBUF
    generic map(
      LOC => "PAD187"
    )
    port map (
      I => ATTENUATION_5_OBUF_6380,
      O => ATTENUATION(5)
    );
  ATTENUATION_6_OBUF : X_OBUF
    generic map(
      LOC => "PAD188"
    )
    port map (
      I => ATTENUATION_6_OBUF_6381,
      O => ATTENUATION(6)
    );
  AMP_PWDN_OBUF : X_OBUF
    generic map(
      LOC => "PAD189"
    )
    port map (
      I => SPI_AMP_SHDN_5830,
      O => AMP_PWDN
    );
  REF_MIX1_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD1"
    )
    port map (
      I => '1',
      O => REF_MIX1_EN
    );
  REF_MIX2_EN_OBUF : X_OBUF
    generic map(
      LOC => "PAD2"
    )
    port map (
      I => '0',
      O => REF_MIX2_EN
    );
  LO1_MOSI_OBUF : X_OBUF
    generic map(
      LOC => "PAD134"
    )
    port map (
      I => LO1_MOSI_OBUF_5926,
      O => LO1_MOSI
    );
  LEDS_0_OBUF : X_OBUF
    generic map(
      LOC => "PAD74"
    )
    port map (
      I => SPI_LEDS(2),
      O => LEDS(0)
    );
  LEDS_1_OBUF : X_OBUF
    generic map(
      LOC => "PAD72"
    )
    port map (
      I => LEDS_1_OBUF_5931,
      O => LEDS(1)
    );
  LEDS_2_OBUF : X_OBUF
    generic map(
      LOC => "PAD71"
    )
    port map (
      I => LEDS_2_OBUF_0,
      O => LEDS(2)
    );
  LEDS_3_OBUF : X_OBUF
    generic map(
      LOC => "PAD73"
    )
    port map (
      I => LEDS_3_OBUF_0,
      O => LEDS(3)
    );
  LEDS_4_OBUF : X_OBUF
    generic map(
      LOC => "PAD75"
    )
    port map (
      I => LEDS_4_OBUF_0,
      O => LEDS(4)
    );
  LEDS_5_OBUF : X_OBUF
    generic map(
      LOC => "PAD76"
    )
    port map (
      I => LEDS_5_OBUF_5836,
      O => LEDS(5)
    );
  LEDS_6_OBUF : X_OBUF
    generic map(
      LOC => "PAD79"
    )
    port map (
      I => SPI_LEDS(0),
      O => LEDS(6)
    );
  LEDS_7_OBUF : X_OBUF
    generic map(
      LOC => "PAD80"
    )
    port map (
      I => SPI_LEDS(1),
      O => LEDS(7)
    );
  LO1_CLK_OBUF : X_OBUF
    generic map(
      LOC => "PAD133"
    )
    port map (
      I => LO1_CLK_OBUF_0,
      O => LO1_CLK
    );
  MCU_AUX1_IBUF : X_BUF
    generic map(
      LOC => "PAD84",
      PATHPULSE => 115 ps
    )
    port map (
      O => MCU_AUX1_IBUF_1055,
      I => MCU_AUX1
    );
  ProtoComp43_IMUX_7 : X_BUF
    generic map(
      LOC => "PAD84",
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_AUX1_IBUF_1055,
      O => MCU_AUX1_IBUF_0
    );
  MCU_AUX2_IBUF : X_BUF
    generic map(
      LOC => "PAD99",
      PATHPULSE => 115 ps
    )
    port map (
      O => MCU_AUX2_IBUF_1058,
      I => MCU_AUX2
    );
  ProtoComp43_IMUX_8 : X_BUF
    generic map(
      LOC => "PAD99",
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_AUX2_IBUF_1058,
      O => MCU_AUX2_IBUF_0
    );
  MCU_AUX3_IBUF : X_BUF
    generic map(
      LOC => "PAD100",
      PATHPULSE => 115 ps
    )
    port map (
      O => MCU_AUX3_IBUF_1061,
      I => MCU_AUX3
    );
  ProtoComp43_IMUX_9 : X_BUF
    generic map(
      LOC => "PAD100",
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_AUX3_IBUF_1061,
      O => MCU_AUX3_IBUF_0
    );
  LO1_MUX_IBUF : X_BUF
    generic map(
      LOC => "PAD121",
      PATHPULSE => 115 ps
    )
    port map (
      O => LO1_MUX_IBUF_1064,
      I => LO1_MUX
    );
  ProtoComp43_IMUX_10 : X_BUF
    generic map(
      LOC => "PAD121",
      PATHPULSE => 115 ps
    )
    port map (
      I => LO1_MUX_IBUF_1064,
      O => LO1_MUX_IBUF_0
    );
  MCU_INTR_OBUF : X_OBUF
    generic map(
      LOC => "PAD110"
    )
    port map (
      I => SPI_INTERRUPT_ASSERTED_6389,
      O => MCU_INTR
    );
  MCU_MISO_OBUFT : X_OBUFT
    generic map(
      LOC => "PAD107"
    )
    port map (
      I => MCU_MISO_OBUFT_0,
      O => MCU_MISO,
      CTL => aux1_sync_aux2_sync_OR_8_o_inv_0
    );
  MCU_MOSI_IBUF : X_BUF
    generic map(
      LOC => "PAD108",
      PATHPULSE => 115 ps
    )
    port map (
      O => MCU_MOSI_IBUF_1072,
      I => MCU_MOSI
    );
  ProtoComp43_IMUX_11 : X_BUF
    generic map(
      LOC => "PAD108",
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_MOSI_IBUF_1072,
      O => MCU_MOSI_IBUF_0
    );
  MCU_SCK_IBUF : X_BUF
    generic map(
      LOC => "PAD104",
      PATHPULSE => 115 ps
    )
    port map (
      O => MCU_SCK_IBUF_1075,
      I => MCU_SCK
    );
  ProtoComp43_IMUX_12 : X_BUF
    generic map(
      LOC => "PAD104",
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_1075,
      O => MCU_SCK_IBUF_0
    );
  MCU_NSS_IBUF : X_BUF
    generic map(
      LOC => "PAD103",
      PATHPULSE => 115 ps
    )
    port map (
      O => MCU_NSS_IBUF_1078,
      I => MCU_NSS
    );
  ProtoComp43_IMUX_13 : X_BUF
    generic map(
      LOC => "PAD103",
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_NSS_IBUF_1078,
      O => MCU_NSS_IBUF_0
    );
  REF_SCLK_OBUF : X_OBUF
    generic map(
      LOC => "PAD200"
    )
    port map (
      I => '0',
      O => REF_SCLK
    );
  REF_SDO_IBUF : X_BUF
    generic map(
      LOC => "PAD199",
      PATHPULSE => 115 ps
    )
    port map (
      O => REF_SDO_IBUF_1083,
      I => REF_SDO
    );
  ProtoComp43_IMUX_14 : X_BUF
    generic map(
      LOC => "PAD199",
      PATHPULSE => 115 ps
    )
    port map (
      I => REF_SDO_IBUF_1083,
      O => REF_SDO_IBUF_0
    );
  PORT_SELECT1_OBUF : X_OBUF
    generic map(
      LOC => "PAD191"
    )
    port map (
      I => PORT_SELECT1_OBUF_5828,
      O => PORT_SELECT1
    );
  PORT_SELECT2_OBUF : X_OBUF
    generic map(
      LOC => "PAD190"
    )
    port map (
      I => PORT_SELECT2_OBUF_0,
      O => PORT_SELECT2
    );
  MainCLK_clkf_buf : X_CKBUF
    generic map(
      LOC => "BUFGMUX_X2Y3",
      PATHPULSE => 115 ps
    )
    port map (
      I => MainCLK_clk0,
      O => MainCLK_clkfb
    );
  MainCLK_dcm_sp_inst_PSENINV : X_BUF
    generic map(
      LOC => "DCM_X0Y1",
      PATHPULSE => 115 ps
    )
    port map (
      I => GND,
      O => MainCLK_dcm_sp_inst_PSEN_INT
    );
  MainCLK_dcm_sp_inst_CLKFB : X_BUF
    generic map(
      LOC => "DCM_X0Y1",
      PATHPULSE => 115 ps
    )
    port map (
      I => MainCLK_dcm_sp_inst_ML_NEW_O,
      O => MainCLK_dcm_sp_inst_CLKFB_INT
    );
  MainCLK_dcm_sp_inst_CLKIN : X_BUF
    generic map(
      LOC => "DCM_X0Y1",
      PATHPULSE => 115 ps
    )
    port map (
      I => MainCLK_dcm_sp_inst_ML_NEW_DIVCLK,
      O => MainCLK_dcm_sp_inst_CLKIN_INT
    );
  MainCLK_dcm_sp_inst_PSINCDECINV : X_BUF
    generic map(
      LOC => "DCM_X0Y1",
      PATHPULSE => 115 ps
    )
    port map (
      I => GND,
      O => MainCLK_dcm_sp_inst_PSINCDEC_INT
    );
  MainCLK_dcm_sp_inst_PSCLKINV : X_BUF
    generic map(
      LOC => "DCM_X0Y1",
      PATHPULSE => 115 ps
    )
    port map (
      I => GND,
      O => MainCLK_dcm_sp_inst_PSCLK_INT
    );
  MainCLK_dcm_sp_inst : X_DCM_SP
    generic map(
      CLKDV_DIVIDE => 2.000000,
      DLL_FREQUENCY_MODE => "LOW",
      DFS_FREQUENCY_MODE => "LOW",
      CLKIN_DIVIDE_BY_2 => FALSE,
      CLKOUT_PHASE_SHIFT => "NONE",
      CLK_FEEDBACK => "1X",
      STARTUP_WAIT => FALSE,
      DSS_MODE => "NONE",
      DUTY_CYCLE_CORRECTION => TRUE,
      DESKEW_ADJUST => "5",
      CLKFX_MULTIPLY => 32,
      CLKFX_DIVIDE => 5,
      PHASE_SHIFT => 0,
      CLKIN_PERIOD => 62.500000,
      LOC => "DCM_X0Y1"
    )
    port map (
      PSCLK => MainCLK_dcm_sp_inst_PSCLK_INT,
      RST => RESET_IBUF_0,
      PSINCDEC => MainCLK_dcm_sp_inst_PSINCDEC_INT,
      CLKIN => MainCLK_dcm_sp_inst_CLKIN_INT,
      CLKFB => MainCLK_dcm_sp_inst_CLKFB_INT,
      PSEN => MainCLK_dcm_sp_inst_PSEN_INT,
      DSSEN => MainCLK_dcm_sp_inst_DSSEN,
      CLK2X180 => MainCLK_dcm_sp_inst_CLK2X180,
      CLK2X => MainCLK_dcm_sp_inst_CLK2X,
      CLKFX => MainCLK_clkfx,
      CLK180 => MainCLK_dcm_sp_inst_CLK180,
      CLK270 => MainCLK_dcm_sp_inst_CLK270,
      CLK0 => MainCLK_clk0,
      CLKFX180 => MainCLK_dcm_sp_inst_CLKFX180,
      CLKDV => MainCLK_dcm_sp_inst_CLKDV,
      PSDONE => MainCLK_dcm_sp_inst_PSDONE,
      CLK90 => MainCLK_dcm_sp_inst_CLK90,
      LOCKED => LEDS_1_OBUF_5931,
      STATUS(7) => MainCLK_dcm_sp_inst_STATUS7,
      STATUS(6) => MainCLK_dcm_sp_inst_STATUS6,
      STATUS(5) => MainCLK_dcm_sp_inst_STATUS5,
      STATUS(4) => MainCLK_dcm_sp_inst_STATUS4,
      STATUS(3) => MainCLK_dcm_sp_inst_STATUS3,
      STATUS(2) => MainCLK_dcm_sp_inst_STATUS2,
      STATUS(1) => MainCLK_dcm_sp_inst_STATUS1,
      STATUS(0) => MainCLK_dcm_sp_inst_STATUS0
    );
  SP6_BUFIO_INSERT_ML_BUFIO2_0 : X_BUFIO2
    generic map(
      DIVIDE => 1,
      DIVIDE_BYPASS => TRUE,
      I_INVERT => FALSE,
      USE_DOUBLER => FALSE,
      LOC => "BUFIO2_X1Y1"
    )
    port map (
      DIVCLK => MainCLK_dcm_sp_inst_ML_NEW_DIVCLK,
      I => MainCLK_clkin1_0,
      IOCLK => NLW_SP6_BUFIO_INSERT_ML_BUFIO2_0_IOCLK_UNCONNECTED,
      SERDESSTROBE => NLW_SP6_BUFIO_INSERT_ML_BUFIO2_0_SERDESSTROBE_UNCONNECTED
    );
  SP6_INS_BUFIO2FB_DCM_ML_BUFIO2FB_1 : X_BUFIO2FB
    generic map(
      DIVIDE_BYPASS => TRUE,
      LOC => "BUFIO2FB_X1Y1"
    )
    port map (
      I => MainCLK_clkfb,
      O => MainCLK_dcm_sp_inst_ML_NEW_O
    );
  MCU_SCK_IBUF_BUFG : X_CKBUF
    generic map(
      LOC => "BUFGMUX_X2Y2",
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_0,
      O => MCU_SCK_IBUF_BUFG_5814
    );
  MainCLK_clkout1_buf : X_CKBUF
    generic map(
      LOC => "BUFGMUX_X2Y1",
      PATHPULSE => 115 ps
    )
    port map (
      I => MainCLK_clkfx,
      O => clk_pll
    );
  Source_latched_regs_19 : X_FF
    generic map(
      LOC => "SLICE_X0Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_19_dpot_1109,
      O => Source_latched_regs(19),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_19_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X0Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(18),
      ADR2 => mod_lookup_read_data(19),
      ADR3 => Source_latched_regs(19),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_19_dpot_1109
    );
  Source_n0148_inv11_rstpot_1 : X_LUT6
    generic map(
      LOC => "SLICE_X0Y30",
      INIT => X"F0F0F0F00000CC00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_sclk_5839,
      ADR2 => source_reload_5964,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => Source_done_int_5965,
      O => Source_n0148_inv11_rstpot1
    );
  Source_latched_regs_18 : X_FF
    generic map(
      LOC => "SLICE_X0Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_18_dpot_1124,
      O => Source_latched_regs(18),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_18_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X0Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(17),
      ADR2 => mod_lookup_read_data(18),
      ADR3 => Source_latched_regs(18),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_18_dpot_1124
    );
  Source_latched_regs_17 : X_FF
    generic map(
      LOC => "SLICE_X0Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_17_dpot_1131,
      O => Source_latched_regs(17),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_17_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X0Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(16),
      ADR2 => mod_lookup_read_data(17),
      ADR3 => Source_latched_regs(17),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_17_dpot_1131
    );
  Source_latched_regs_82 : X_FF
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_82_dpot_1158,
      O => Source_latched_regs(82),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_82_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(81),
      ADR2 => mod_lookup_read_data(82),
      ADR3 => Source_latched_regs(82),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_82_dpot_1158
    );
  Source_latched_regs_81 : X_FF
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_81_dpot_1137,
      O => Source_latched_regs(81),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_81_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(80),
      ADR2 => mod_lookup_read_data(81),
      ADR3 => Source_latched_regs(81),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_81_dpot_1137
    );
  Source_latched_regs_80 : X_FF
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_80_dpot_1144,
      O => Source_latched_regs(80),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_80_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(79),
      ADR2 => mod_lookup_read_data(80),
      ADR3 => Source_latched_regs(80),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_80_dpot_1144
    );
  Source_latched_regs_79 : X_FF
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_79_dpot_1151,
      O => Source_latched_regs(79),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_79_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X0Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(78),
      ADR2 => mod_lookup_read_data(79),
      ADR3 => Source_latched_regs(79),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_79_dpot_1151
    );
  BAND_SELECT_HIGH1_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X1Y33",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => SPI_SOURCE_BANDSELECT_6374,
      O => BAND_SELECT_HIGH_OBUF_6371
    );
  Source_latched_regs_78 : X_FF
    generic map(
      LOC => "SLICE_X1Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_78_dpot_1170,
      O => Source_latched_regs(78),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_78_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X1Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(77),
      ADR2 => mod_lookup_read_data(78),
      ADR3 => Source_latched_regs(78),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_78_dpot_1170
    );
  Source_latched_regs_77 : X_FF
    generic map(
      LOC => "SLICE_X1Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_77_dpot_1179,
      O => Source_latched_regs(77),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_77_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X1Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(76),
      ADR2 => mod_lookup_read_data(77),
      ADR3 => Source_latched_regs(77),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_77_dpot_1179
    );
  Source_latched_regs_76 : X_FF
    generic map(
      LOC => "SLICE_X1Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_76_dpot_1186,
      O => Source_latched_regs(76),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_76_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X1Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(75),
      ADR2 => mod_lookup_read_data(76),
      ADR3 => Source_latched_regs(76),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_76_dpot_1186
    );
  Source_n0148_inv11_rstpot_4 : X_LUT6
    generic map(
      LOC => "SLICE_X1Y38",
      INIT => X"F0F0F0F00000CC00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_sclk_5839,
      ADR2 => source_reload_5964,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => Source_done_int_5965,
      O => Source_n0148_inv11_rstpot4
    );
  PORT_SELECT1_OBUF_PORT_SELECT1_OBUF_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => PORT_SELECT2_OBUF_1199,
      O => PORT_SELECT2_OBUF_0
    );
  PORT1_SELECT1 : X_LUT6
    generic map(
      LOC => "SLICE_X1Y43",
      INIT => X"0000FF000000FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_PORTSELECT_5829,
      ADR4 => SPI_AMP_SHDN_5830,
      ADR5 => '1',
      O => PORT_SELECT1_OBUF_5828
    );
  PORT2_SELECT1 : X_LUT5
    generic map(
      LOC => "SLICE_X1Y43",
      INIT => X"000000FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_PORTSELECT_5829,
      ADR4 => SPI_AMP_SHDN_5830,
      O => PORT_SELECT2_OBUF_1199
    );
  FILT_IN_C21_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y28",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => SPI_SOURCE_FILTER(1),
      O => FILT_IN_C2_OBUF_6372
    );
  Source_latched_regs_23 : X_FF
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_23_dpot_1226,
      O => Source_latched_regs(23),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_23_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(22),
      ADR2 => mod_lookup_read_data(23),
      ADR3 => Source_latched_regs(23),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_23_dpot_1226
    );
  Source_latched_regs_22 : X_FF
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_22_dpot_1205,
      O => Source_latched_regs(22),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_22_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(21),
      ADR2 => mod_lookup_read_data(22),
      ADR3 => Source_latched_regs(22),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_22_dpot_1205
    );
  Source_latched_regs_21 : X_FF
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_21_dpot_1212,
      O => Source_latched_regs(21),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_21_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(20),
      ADR2 => mod_lookup_read_data(21),
      ADR3 => Source_latched_regs(21),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_21_dpot_1212
    );
  Source_latched_regs_20 : X_FF
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_20_dpot_1219,
      O => Source_latched_regs(20),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_20_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y29",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(19),
      ADR2 => mod_lookup_read_data(20),
      ADR3 => Source_latched_regs(20),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_20_dpot_1219
    );
  Source_latched_regs_27 : X_FF
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_27_dpot_1256,
      O => Source_latched_regs(27),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_27_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(26),
      ADR2 => mod_lookup_read_data(27),
      ADR3 => Source_latched_regs(27),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_27_dpot_1256
    );
  Source_latched_regs_26 : X_FF
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_26_dpot_1235,
      O => Source_latched_regs(26),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_26_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(25),
      ADR2 => mod_lookup_read_data(26),
      ADR3 => Source_latched_regs(26),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_26_dpot_1235
    );
  Source_latched_regs_25 : X_FF
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_25_dpot_1242,
      O => Source_latched_regs(25),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_25_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(24),
      ADR2 => mod_lookup_read_data(25),
      ADR3 => Source_latched_regs(25),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_25_dpot_1242
    );
  Source_latched_regs_24 : X_FF
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_24_dpot_1249,
      O => Source_latched_regs(24),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_24_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(23),
      ADR2 => mod_lookup_read_data(24),
      ADR3 => Source_latched_regs(24),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_24_dpot_1249
    );
  Source_latched_regs_16 : X_FF
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_16_dpot_1286,
      O => Source_latched_regs(16),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_16_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(15),
      ADR2 => mod_lookup_read_data(16),
      ADR3 => Source_latched_regs(16),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_16_dpot_1286
    );
  Source_latched_regs_15 : X_FF
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_15_dpot_1265,
      O => Source_latched_regs(15),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_15_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(14),
      ADR2 => mod_lookup_read_data(15),
      ADR3 => Source_latched_regs(15),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_15_dpot_1265
    );
  Source_latched_regs_14 : X_FF
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_14_dpot_1272,
      O => Source_latched_regs(14),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_14_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(13),
      ADR2 => mod_lookup_read_data(14),
      ADR3 => Source_latched_regs(14),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_14_dpot_1272
    );
  Source_latched_regs_13 : X_FF
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_13_dpot_1279,
      O => Source_latched_regs(13),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_13_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(12),
      ADR2 => mod_lookup_read_data(13),
      ADR3 => Source_latched_regs(13),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_13_dpot_1279
    );
  source_reloaded_source_reloaded_last_AND_35_o_source_reloaded_source_reloaded_last_AND_35_o_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Source_latched_regs_0_pack_4,
      O => Source_latched_regs(0)
    );
  source_reloaded_source_reloaded_last_AND_35_o1 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y32",
      INIT => X"00FF000000FF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => source_reloaded_last_6662,
      ADR4 => Source_done_int_5965,
      ADR5 => '1',
      O => source_reloaded_source_reloaded_last_AND_35_o
    );
  Source_latched_regs_0_dpot : X_LUT5
    generic map(
      LOC => "SLICE_X2Y32",
      INIT => X"CACA0A0A"
    )
    port map (
      ADR0 => Source_latched_regs(0),
      ADR1 => mod_lookup_read_data(0),
      ADR2 => Source_n0148_inv11_rstpot_6449,
      ADR3 => '1',
      ADR4 => Source_done_int_5965,
      O => Source_latched_regs_0_dpot_1306
    );
  Source_latched_regs_0 : X_FF
    generic map(
      LOC => "SLICE_X2Y32",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_0_dpot_1306,
      O => Source_latched_regs_0_pack_4,
      RST => GND,
      SET => GND
    );
  Source_n0148_inv11_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y32",
      INIT => X"F0F0F0F00000CC00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_sclk_5839,
      ADR2 => source_reload_5964,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => Source_done_int_5965,
      O => Source_n0148_inv11_rstpot_6449
    );
  Source_n0148_inv11_cepot_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y32",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => Source_n0148_inv11_cepot
    );
  SPI_SOURCE_BANDSELECT : X_FF
    generic map(
      LOC => "SLICE_X2Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_BANDSELECT_dpot_1318,
      O => SPI_SOURCE_BANDSELECT_6374,
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_BANDSELECT_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y33",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_BANDSELECT_6374,
      ADR4 => SPI_SPI_BUF_OUT(4),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_BANDSELECT_dpot_1318
    );
  ATTENUATION_2_1 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y35",
      INIT => X"F0F0FFF0F0F000F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SOURCE_ATTENUATION(2),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => Sync_AUX3_sync_line(2),
      ADR5 => mod_attenuator(2),
      O => ATTENUATION_2_OBUF_6377
    );
  Source_n0148_inv11_cepot_2_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y37",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => Source_n0148_inv11_cepot2
    );
  Source_latched_regs_95 : X_FF
    generic map(
      LOC => "SLICE_X2Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_95_dpot_1331,
      O => Source_latched_regs(95),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_95_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(94),
      ADR2 => mod_lookup_read_data(95),
      ADR3 => Source_latched_regs(95),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_95_dpot_1331
    );
  Source_n0148_inv11_rstpot_5 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y37",
      INIT => X"F0F0F0F00000CC00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_sclk_5839,
      ADR2 => source_reload_5964,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => Source_done_int_5965,
      O => Source_n0148_inv11_rstpot5
    );
  Source_latched_regs_99 : X_FF
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_99_dpot_1364,
      O => Source_latched_regs(99),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_99_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(98),
      ADR2 => mod_lookup_read_data(99),
      ADR3 => Source_latched_regs(99),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_99_dpot_1364
    );
  Source_latched_regs_98 : X_FF
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_98_dpot_1343,
      O => Source_latched_regs(98),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_98_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(97),
      ADR2 => mod_lookup_read_data(98),
      ADR3 => Source_latched_regs(98),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_98_dpot_1343
    );
  Source_latched_regs_97 : X_FF
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_97_dpot_1350,
      O => Source_latched_regs(97),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_97_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(96),
      ADR2 => mod_lookup_read_data(97),
      ADR3 => Source_latched_regs(97),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_97_dpot_1350
    );
  Source_latched_regs_96 : X_FF
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_96_dpot_1357,
      O => Source_latched_regs(96),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_96_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(95),
      ADR2 => mod_lookup_read_data(96),
      ADR3 => Source_latched_regs(96),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_96_dpot_1357
    );
  Source_latched_regs_90 : X_FF
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_90_dpot_1394,
      O => Source_latched_regs(90),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_90_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(89),
      ADR2 => mod_lookup_read_data(90),
      ADR3 => Source_latched_regs(90),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_90_dpot_1394
    );
  Source_latched_regs_89 : X_FF
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_89_dpot_1373,
      O => Source_latched_regs(89),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_89_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(88),
      ADR2 => mod_lookup_read_data(89),
      ADR3 => Source_latched_regs(89),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_89_dpot_1373
    );
  Source_latched_regs_88 : X_FF
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_88_dpot_1380,
      O => Source_latched_regs(88),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_88_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(87),
      ADR2 => mod_lookup_read_data(88),
      ADR3 => Source_latched_regs(88),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_88_dpot_1380
    );
  Source_latched_regs_87 : X_FF
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_87_dpot_1387,
      O => Source_latched_regs(87),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_87_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(86),
      ADR2 => mod_lookup_read_data(87),
      ADR3 => Source_latched_regs(87),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_87_dpot_1387
    );
  ATTENUATION_4_1 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y44",
      INIT => X"F0F0FFF0F0F000F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SOURCE_ATTENUATION(4),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => Sync_AUX3_sync_line(2),
      ADR5 => mod_attenuator(4),
      O => ATTENUATION_4_OBUF_6379
    );
  ATTENUATION_6_1 : X_LUT6
    generic map(
      LOC => "SLICE_X2Y44",
      INIT => X"F0F0FFF0F0F000F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SOURCE_ATTENUATION(6),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => Sync_AUX3_sync_line(2),
      ADR5 => mod_attenuator(6),
      O => ATTENUATION_6_OBUF_6381
    );
  Source_latched_regs_119 : X_FF
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_119_dpot_1434,
      O => Source_latched_regs(119),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_119_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(118),
      ADR2 => mod_lookup_read_data(119),
      ADR3 => Source_latched_regs(119),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_119_dpot_1434
    );
  Source_latched_regs_118 : X_FF
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_118_dpot_1413,
      O => Source_latched_regs(118),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_118_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(117),
      ADR2 => mod_lookup_read_data(118),
      ADR3 => Source_latched_regs(118),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_118_dpot_1413
    );
  Source_latched_regs_117 : X_FF
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_117_dpot_1420,
      O => Source_latched_regs(117),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_117_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(116),
      ADR2 => mod_lookup_read_data(117),
      ADR3 => Source_latched_regs(117),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_117_dpot_1420
    );
  Source_latched_regs_116 : X_FF
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_116_dpot_1427,
      O => Source_latched_regs(116),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_116_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X2Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(115),
      ADR2 => mod_lookup_read_data(116),
      ADR3 => Source_latched_regs(116),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_116_dpot_1427
    );
  SPI_SOURCE_FILTER_1 : X_FF
    generic map(
      LOC => "SLICE_X3Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_FILTER_1_dpot_1447,
      O => SPI_SOURCE_FILTER(1),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_FILTER_1_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y29",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_FILTER(1),
      ADR4 => SPI_SPI_BUF_OUT(15),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_FILTER_1_dpot_1447
    );
  Source_latched_regs_31 : X_FF
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_31_dpot_1471,
      O => Source_latched_regs(31),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_31_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(30),
      ADR2 => mod_lookup_read_data(31),
      ADR3 => Source_latched_regs(31),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_31_dpot_1471
    );
  Source_latched_regs_30 : X_FF
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_30_dpot_1450,
      O => Source_latched_regs(30),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_30_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(29),
      ADR2 => mod_lookup_read_data(30),
      ADR3 => Source_latched_regs(30),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_30_dpot_1450
    );
  Source_latched_regs_29 : X_FF
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_29_dpot_1457,
      O => Source_latched_regs(29),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_29_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(28),
      ADR2 => mod_lookup_read_data(29),
      ADR3 => Source_latched_regs(29),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_29_dpot_1457
    );
  Source_latched_regs_28 : X_FF
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_28_dpot_1464,
      O => Source_latched_regs(28),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_28_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(27),
      ADR2 => mod_lookup_read_data(28),
      ADR3 => Source_latched_regs(28),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_28_dpot_1464
    );
  Source_latched_regs_12 : X_FF
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_12_dpot_1501,
      O => Source_latched_regs(12),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_12_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(11),
      ADR2 => mod_lookup_read_data(12),
      ADR3 => Source_latched_regs(12),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_12_dpot_1501
    );
  Source_latched_regs_11 : X_FF
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_11_dpot_1480,
      O => Source_latched_regs(11),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_11_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(10),
      ADR2 => mod_lookup_read_data(11),
      ADR3 => Source_latched_regs(11),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_11_dpot_1480
    );
  Source_latched_regs_10 : X_FF
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_10_dpot_1487,
      O => Source_latched_regs(10),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_10_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(9),
      ADR2 => mod_lookup_read_data(10),
      ADR3 => Source_latched_regs(10),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_10_dpot_1487
    );
  Source_latched_regs_9 : X_FF
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_9_dpot_1494,
      O => Source_latched_regs(9),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_9_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(8),
      ADR2 => mod_lookup_read_data(9),
      ADR3 => Source_latched_regs(9),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_9_dpot_1494
    );
  source_reloaded_last : X_FF
    generic map(
      LOC => "SLICE_X3Y32",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Source_done_int_5965,
      O => source_reloaded_last_6662,
      RST => GND,
      SET => GND
    );
  ATTENUATION_0_1 : X_LUT6
    generic map(
      LOC => "SLICE_X3Y34",
      INIT => X"F0F0FFF0F0F000F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SOURCE_ATTENUATION(0),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => Sync_AUX3_sync_line(2),
      ADR5 => mod_attenuator(0),
      O => ATTENUATION_0_OBUF_6375
    );
  ATTENUATION_1_1 : X_LUT6
    generic map(
      LOC => "SLICE_X3Y35",
      INIT => X"F0F0FFF0F0F000F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SOURCE_ATTENUATION(1),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => Sync_AUX3_sync_line(2),
      ADR5 => mod_attenuator(1),
      O => ATTENUATION_1_OBUF_6376
    );
  Source_latched_regs_94 : X_FF
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_94_dpot_1544,
      O => Source_latched_regs(94),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_94_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(93),
      ADR2 => mod_lookup_read_data(94),
      ADR3 => Source_latched_regs(94),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_94_dpot_1544
    );
  Source_latched_regs_93 : X_FF
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_93_dpot_1523,
      O => Source_latched_regs(93),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_93_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(92),
      ADR2 => mod_lookup_read_data(93),
      ADR3 => Source_latched_regs(93),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_93_dpot_1523
    );
  Source_latched_regs_92 : X_FF
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_92_dpot_1530,
      O => Source_latched_regs(92),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_92_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(91),
      ADR2 => mod_lookup_read_data(92),
      ADR3 => Source_latched_regs(92),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_92_dpot_1530
    );
  Source_latched_regs_91 : X_FF
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_91_dpot_1537,
      O => Source_latched_regs(91),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_91_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(90),
      ADR2 => mod_lookup_read_data(91),
      ADR3 => Source_latched_regs(91),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_91_dpot_1537
    );
  Source_latched_regs_86 : X_FF
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_86_dpot_1574,
      O => Source_latched_regs(86),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_86_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(85),
      ADR2 => mod_lookup_read_data(86),
      ADR3 => Source_latched_regs(86),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_86_dpot_1574
    );
  Source_latched_regs_85 : X_FF
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_85_dpot_1553,
      O => Source_latched_regs(85),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_85_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(84),
      ADR2 => mod_lookup_read_data(85),
      ADR3 => Source_latched_regs(85),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_85_dpot_1553
    );
  Source_latched_regs_84 : X_FF
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_84_dpot_1560,
      O => Source_latched_regs(84),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_84_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(83),
      ADR2 => mod_lookup_read_data(84),
      ADR3 => Source_latched_regs(84),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_84_dpot_1560
    );
  Source_latched_regs_83 : X_FF
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_83_dpot_1567,
      O => Source_latched_regs(83),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_83_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y39",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(82),
      ADR2 => mod_lookup_read_data(83),
      ADR3 => Source_latched_regs(83),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot4,
      O => Source_latched_regs_83_dpot_1567
    );
  Source_n0148_inv11_cepot_3_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X3Y42",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => Source_n0148_inv11_cepot3
    );
  mod_attenuator_3 : X_FF
    generic map(
      LOC => "SLICE_X3Y43",
      INIT => '0'
    )
    port map (
      CE => source_reloaded_source_reloaded_last_AND_35_o,
      CLK => clk_pll,
      I => mod_lookup_read_data(131),
      O => mod_attenuator(3),
      RST => GND,
      SET => GND
    );
  mod_attenuator_2 : X_FF
    generic map(
      LOC => "SLICE_X3Y43",
      INIT => '0'
    )
    port map (
      CE => source_reloaded_source_reloaded_last_AND_35_o,
      CLK => clk_pll,
      I => mod_lookup_read_data(130),
      O => mod_attenuator(2),
      RST => GND,
      SET => GND
    );
  mod_attenuator_1 : X_FF
    generic map(
      LOC => "SLICE_X3Y43",
      INIT => '0'
    )
    port map (
      CE => source_reloaded_source_reloaded_last_AND_35_o,
      CLK => clk_pll,
      I => mod_lookup_read_data(129),
      O => mod_attenuator(1),
      RST => GND,
      SET => GND
    );
  ATTENUATION_3_1 : X_LUT6
    generic map(
      LOC => "SLICE_X3Y43",
      INIT => X"F0F0FFF0F0F000F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SOURCE_ATTENUATION(3),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => Sync_AUX3_sync_line(2),
      ADR5 => mod_attenuator(3),
      O => ATTENUATION_3_OBUF_6378
    );
  mod_attenuator_0 : X_FF
    generic map(
      LOC => "SLICE_X3Y43",
      INIT => '0'
    )
    port map (
      CE => source_reloaded_source_reloaded_last_AND_35_o,
      CLK => clk_pll,
      I => mod_lookup_read_data(128),
      O => mod_attenuator(0),
      RST => GND,
      SET => GND
    );
  mod_attenuator_6 : X_FF
    generic map(
      LOC => "SLICE_X3Y44",
      INIT => '0'
    )
    port map (
      CE => source_reloaded_source_reloaded_last_AND_35_o,
      CLK => clk_pll,
      I => mod_lookup_read_data(134),
      O => mod_attenuator(6),
      RST => GND,
      SET => GND
    );
  ATTENUATION_5_1 : X_LUT6
    generic map(
      LOC => "SLICE_X3Y44",
      INIT => X"F0F0FFF0F0F000F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SOURCE_ATTENUATION(5),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => Sync_AUX3_sync_line(2),
      ADR5 => mod_attenuator(5),
      O => ATTENUATION_5_OBUF_6380
    );
  mod_attenuator_5 : X_FF
    generic map(
      LOC => "SLICE_X3Y44",
      INIT => '0'
    )
    port map (
      CE => source_reloaded_source_reloaded_last_AND_35_o,
      CLK => clk_pll,
      I => mod_lookup_read_data(133),
      O => mod_attenuator(5),
      RST => GND,
      SET => GND
    );
  mod_attenuator_4 : X_FF
    generic map(
      LOC => "SLICE_X3Y44",
      INIT => '0'
    )
    port map (
      CE => source_reloaded_source_reloaded_last_AND_35_o,
      CLK => clk_pll,
      I => mod_lookup_read_data(132),
      O => mod_attenuator(4),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_123 : X_FF
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_123_dpot_1634,
      O => Source_latched_regs(123),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_123_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(122),
      ADR2 => mod_lookup_read_data(123),
      ADR3 => Source_latched_regs(123),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_123_dpot_1634
    );
  Source_latched_regs_122 : X_FF
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_122_dpot_1613,
      O => Source_latched_regs(122),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_122_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(121),
      ADR2 => mod_lookup_read_data(122),
      ADR3 => Source_latched_regs(122),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_122_dpot_1613
    );
  Source_latched_regs_121 : X_FF
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_121_dpot_1620,
      O => Source_latched_regs(121),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_121_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(120),
      ADR2 => mod_lookup_read_data(121),
      ADR3 => Source_latched_regs(121),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_121_dpot_1620
    );
  Source_latched_regs_120 : X_FF
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_120_dpot_1627,
      O => Source_latched_regs(120),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_120_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X3Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(119),
      ADR2 => mod_lookup_read_data(120),
      ADR3 => Source_latched_regs(120),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_120_dpot_1627
    );
  SPI_MOD_PHASE_INC_14 : X_FF
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_14_dpot1_1650,
      O => SPI_MOD_PHASE_INC(14),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_14_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(14),
      ADR4 => SPI_MOD_PHASE_INC_14_dpot_6457,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_14_dpot1_1650
    );
  SPI_MOD_PHASE_INC_13 : X_FF
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_13_dpot1_1657,
      O => SPI_MOD_PHASE_INC(13),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_13_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(13),
      ADR4 => SPI_MOD_PHASE_INC_13_dpot_6456,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_13_dpot1_1657
    );
  SPI_MOD_PHASE_INC_12 : X_FF
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_12_dpot1_1662,
      O => SPI_MOD_PHASE_INC(12),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_12_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(12),
      ADR4 => SPI_MOD_PHASE_INC_12_dpot_6455,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_12_dpot1_1662
    );
  SPI_MOD_PHASE_INC_11 : X_FF
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_11_dpot1_1645,
      O => SPI_MOD_PHASE_INC(11),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_11_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X4Y12",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(11),
      ADR4 => SPI_MOD_PHASE_INC_11_dpot_6454,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_11_dpot1_1645
    );
  SPI_SOURCE_RF_EN : X_SFF
    generic map(
      LOC => "SLICE_X4Y14",
      INIT => '0'
    )
    port map (
      CE => SPI_n0484_inv_6653,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(1),
      O => SPI_SOURCE_RF_EN_6369,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_MOD_LOOKUP_DATA_3 : X_FF
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_3_dpot_1671,
      O => SPI_MOD_LOOKUP_DATA(3),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_3_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(3),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(3),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_3_dpot_1671
    );
  SPI_MOD_LOOKUP_DATA_2 : X_FF
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_2_dpot_1680,
      O => SPI_MOD_LOOKUP_DATA(2),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_2_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(2),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(2),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_2_dpot_1680
    );
  SPI_MOD_LOOKUP_DATA_1 : X_FF
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_1_dpot_1687,
      O => SPI_MOD_LOOKUP_DATA(1),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_1_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(1),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(1),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_1_dpot_1687
    );
  SPI_MOD_LOOKUP_DATA_0 : X_FF
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_0_dpot_1694,
      O => SPI_MOD_LOOKUP_DATA(0),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_0_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(0),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(0),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_0_dpot_1694
    );
  Source_latched_regs_8 : X_FF
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_8_dpot_1721,
      O => Source_latched_regs(8),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_8_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(7),
      ADR2 => mod_lookup_read_data(8),
      ADR3 => Source_latched_regs(8),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_8_dpot_1721
    );
  Source_latched_regs_7 : X_FF
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_7_dpot_1700,
      O => Source_latched_regs(7),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_7_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(6),
      ADR2 => mod_lookup_read_data(7),
      ADR3 => Source_latched_regs(7),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_7_dpot_1700
    );
  Source_latched_regs_6 : X_FF
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_6_dpot_1707,
      O => Source_latched_regs(6),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_6_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(5),
      ADR2 => mod_lookup_read_data(6),
      ADR3 => Source_latched_regs(6),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_6_dpot_1707
    );
  Source_latched_regs_5 : X_FF
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_5_dpot_1714,
      O => Source_latched_regs(5),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_5_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(4),
      ADR2 => mod_lookup_read_data(5),
      ADR3 => Source_latched_regs(5),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_5_dpot_1714
    );
  Source_sclk_Source_sclk_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Source_n0148_inv11_rstpot6,
      O => Source_n0148_inv11_rstpot6_0
    );
  Source_sclk_Source_sclk_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Source_done_int_glue_rst_pack_13,
      O => Source_done_int_glue_rst_6792
    );
  Source_sclk : X_SFF
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Source_sclk_rstpot_1751,
      O => Source_sclk_5839,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_sclk_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => X"FF00CF30FF00CF30"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_bit_cnt(5),
      ADR2 => Source_clk_cnt(2),
      ADR3 => Source_sclk_5839,
      ADR4 => Source_done_int_5965,
      ADR5 => '1',
      O => Source_sclk_rstpot_1751
    );
  Source_n0148_inv11_rstpot_6 : X_LUT5
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => X"AAAA3000"
    )
    port map (
      ADR0 => source_reload_5964,
      ADR1 => Source_bit_cnt(5),
      ADR2 => Source_clk_cnt(2),
      ADR3 => Source_sclk_5839,
      ADR4 => Source_done_int_5965,
      O => Source_n0148_inv11_rstpot6
    );
  Source_latch : X_SFF
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Source_latch_rstpot_1738,
      O => Source_latch_5842,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_latch_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => X"FF00FF000FF0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Source_bit_cnt(5),
      ADR3 => Source_latch_5842,
      ADR4 => Source_clk_cnt(2),
      ADR5 => Source_done_int_5965,
      O => Source_latch_rstpot_1738
    );
  Source_done_int : X_SFF
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Source_done_int_glue_rst_6792,
      O => Source_done_int_5965,
      SSET => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Source_n0185_inv1 : X_LUT6
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => X"000000FF000000FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Source_done_int_5965,
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => '1',
      O => Source_n0185_inv
    );
  Source_done_int_glue_rst : X_LUT5
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => X"FFF033F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => source_reload_5964,
      ADR2 => Source_n0204_inv,
      ADR3 => Source_done_int_5965,
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      O => Source_done_int_glue_rst_pack_13
    );
  Source_done_int_1 : X_SFF
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Source_done_int_glue_rst_6792,
      O => Source_done_int_1_6733,
      SSET => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Source_n0204_inv11 : X_LUT6
    generic map(
      LOC => "SLICE_X4Y34",
      INIT => X"0000000080000000"
    )
    port map (
      ADR0 => Source_reg_cnt(1),
      ADR1 => Source_reg_cnt(0),
      ADR2 => Source_latch_5842,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => Source_done_int_5965,
      O => Source_n0204_inv
    );
  Source_latched_regs_103 : X_FF
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_103_dpot_1786,
      O => Source_latched_regs(103),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_103_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(102),
      ADR2 => mod_lookup_read_data(103),
      ADR3 => Source_latched_regs(103),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_103_dpot_1786
    );
  Source_latched_regs_102 : X_FF
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_102_dpot_1765,
      O => Source_latched_regs(102),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_102_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(101),
      ADR2 => mod_lookup_read_data(102),
      ADR3 => Source_latched_regs(102),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_102_dpot_1765
    );
  Source_latched_regs_101 : X_FF
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_101_dpot_1772,
      O => Source_latched_regs(101),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_101_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(100),
      ADR2 => mod_lookup_read_data(101),
      ADR3 => Source_latched_regs(101),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_101_dpot_1772
    );
  Source_latched_regs_100 : X_FF
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_100_dpot_1779,
      O => Source_latched_regs(100),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_100_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(99),
      ADR2 => mod_lookup_read_data(100),
      ADR3 => Source_latched_regs(100),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_100_dpot_1779
    );
  SPI_MOD_LOOKUP_DATA_107 : X_FF
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_107_dpot_1806,
      O => SPI_MOD_LOOKUP_DATA(107),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_107_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_91_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(107),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_107_dpot_1806
    );
  SPI_MOD_LOOKUP_DATA_106 : X_FF
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_106_dpot_1815,
      O => SPI_MOD_LOOKUP_DATA(106),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_106_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_90_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(106),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_106_dpot_1815
    );
  SPI_MOD_LOOKUP_DATA_105 : X_FF
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_105_dpot_1822,
      O => SPI_MOD_LOOKUP_DATA(105),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_105_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_89_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(105),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_105_dpot_1822
    );
  SPI_MOD_LOOKUP_DATA_104 : X_FF
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_104_dpot_1799,
      O => SPI_MOD_LOOKUP_DATA(104),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_104_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_88_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(104),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_104_dpot_1799
    );
  Source_latched_regs_107 : X_FF
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_107_dpot_1846,
      O => Source_latched_regs(107),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_107_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(106),
      ADR2 => mod_lookup_read_data(107),
      ADR3 => Source_latched_regs(107),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_107_dpot_1846
    );
  Source_latched_regs_106 : X_FF
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_106_dpot_1825,
      O => Source_latched_regs(106),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_106_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(105),
      ADR2 => mod_lookup_read_data(106),
      ADR3 => Source_latched_regs(106),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_106_dpot_1825
    );
  Source_latched_regs_105 : X_FF
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_105_dpot_1832,
      O => Source_latched_regs(105),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_105_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(104),
      ADR2 => mod_lookup_read_data(105),
      ADR3 => Source_latched_regs(105),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_105_dpot_1832
    );
  Source_latched_regs_104 : X_FF
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_104_dpot_1839,
      O => Source_latched_regs(104),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_104_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y40",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(103),
      ADR2 => mod_lookup_read_data(104),
      ADR3 => Source_latched_regs(104),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_104_dpot_1839
    );
  Source_latched_regs_111 : X_FF
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_111_dpot_1876,
      O => Source_latched_regs(111),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_111_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(110),
      ADR2 => mod_lookup_read_data(111),
      ADR3 => Source_latched_regs(111),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_111_dpot_1876
    );
  Source_latched_regs_110 : X_FF
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_110_dpot_1855,
      O => Source_latched_regs(110),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_110_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(109),
      ADR2 => mod_lookup_read_data(110),
      ADR3 => Source_latched_regs(110),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_110_dpot_1855
    );
  Source_latched_regs_109 : X_FF
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_109_dpot_1862,
      O => Source_latched_regs(109),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_109_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(108),
      ADR2 => mod_lookup_read_data(109),
      ADR3 => Source_latched_regs(109),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_109_dpot_1862
    );
  Source_latched_regs_108 : X_FF
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_108_dpot_1869,
      O => Source_latched_regs(108),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_108_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(107),
      ADR2 => mod_lookup_read_data(108),
      ADR3 => Source_latched_regs(108),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_108_dpot_1869
    );
  Source_latched_regs_127 : X_FF
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_127_dpot_1906,
      O => Source_latched_regs(127),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_127_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(126),
      ADR2 => mod_lookup_read_data(127),
      ADR3 => Source_latched_regs(127),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_127_dpot_1906
    );
  Source_latched_regs_126 : X_FF
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_126_dpot_1885,
      O => Source_latched_regs(126),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_126_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(125),
      ADR2 => mod_lookup_read_data(126),
      ADR3 => Source_latched_regs(126),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_126_dpot_1885
    );
  Source_latched_regs_125 : X_FF
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_125_dpot_1892,
      O => Source_latched_regs(125),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_125_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(124),
      ADR2 => mod_lookup_read_data(125),
      ADR3 => Source_latched_regs(125),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_125_dpot_1892
    );
  Source_latched_regs_124 : X_FF
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_124_dpot_1899,
      O => Source_latched_regs(124),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_124_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X4Y45",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(123),
      ADR2 => mod_lookup_read_data(124),
      ADR3 => Source_latched_regs(124),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_124_dpot_1899
    );
  SPI_MOD_PHASE_INC_14_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y12",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(14),
      ADR1 => SPI_SPI_BUF_OUT(14),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_14_dpot_6457
    );
  SPI_MOD_PHASE_INC_13_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y12",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(13),
      ADR1 => SPI_SPI_BUF_OUT(13),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_13_dpot_6456
    );
  SPI_MOD_PHASE_INC_12_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y13",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(12),
      ADR1 => SPI_SPI_BUF_OUT(12),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_12_dpot_6455
    );
  SPI_SOURCE_CE_EN : X_SFF
    generic map(
      LOC => "SLICE_X5Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_n0484_inv_6653,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(2),
      O => SPI_SOURCE_CE_EN_6366,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_MOD_LOOKUP_DATA_15 : X_FF
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_15_dpot_1942,
      O => SPI_MOD_LOOKUP_DATA(15),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_15_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(15),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(15),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_15_dpot_1942
    );
  SPI_MOD_LOOKUP_DATA_14 : X_FF
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_14_dpot_1951,
      O => SPI_MOD_LOOKUP_DATA(14),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_14_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(14),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(14),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_14_dpot_1951
    );
  SPI_MOD_LOOKUP_DATA_13 : X_FF
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_13_dpot_1958,
      O => SPI_MOD_LOOKUP_DATA(13),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_13_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(13),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(13),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_13_dpot_1958
    );
  SPI_MOD_LOOKUP_DATA_12 : X_FF
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_12_dpot_1965,
      O => SPI_MOD_LOOKUP_DATA(12),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_12_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(12),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(12),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_12_dpot_1965
    );
  Source_latched_regs_4 : X_FF
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_4_dpot_1992,
      O => Source_latched_regs(4),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_4_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(3),
      ADR2 => mod_lookup_read_data(4),
      ADR3 => Source_latched_regs(4),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_4_dpot_1992
    );
  Source_latched_regs_3 : X_FF
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_3_dpot_1971,
      O => Source_latched_regs(3),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_3_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(2),
      ADR2 => mod_lookup_read_data(3),
      ADR3 => Source_latched_regs(3),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_3_dpot_1971
    );
  Source_latched_regs_2 : X_FF
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_2_dpot_1978,
      O => Source_latched_regs(2),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_2_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(1),
      ADR2 => mod_lookup_read_data(2),
      ADR3 => Source_latched_regs(2),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_2_dpot_1978
    );
  Source_latched_regs_1 : X_FF
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot,
      CLK => clk_pll,
      I => Source_latched_regs_1_dpot_1985,
      O => Source_latched_regs(1),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_1_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y30",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(0),
      ADR2 => mod_lookup_read_data(1),
      ADR3 => Source_latched_regs(1),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot_6449,
      O => Source_latched_regs_1_dpot_1985
    );
  Source_latched_regs_35 : X_FF
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_35_dpot_2022,
      O => Source_latched_regs(35),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_35_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(34),
      ADR2 => mod_lookup_read_data(35),
      ADR3 => Source_latched_regs(35),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_35_dpot_2022
    );
  Source_latched_regs_34 : X_FF
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_34_dpot_2001,
      O => Source_latched_regs(34),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_34_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(33),
      ADR2 => mod_lookup_read_data(34),
      ADR3 => Source_latched_regs(34),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_34_dpot_2001
    );
  Source_latched_regs_33 : X_FF
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_33_dpot_2008,
      O => Source_latched_regs(33),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_33_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(32),
      ADR2 => mod_lookup_read_data(33),
      ADR3 => Source_latched_regs(33),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_33_dpot_2008
    );
  Source_latched_regs_32 : X_FF
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_32_dpot_2015,
      O => Source_latched_regs(32),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_32_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y31",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(31),
      ADR2 => mod_lookup_read_data(32),
      ADR3 => Source_latched_regs(32),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_32_dpot_2015
    );
  Source_clk_cnt_2_Source_clk_cnt_2_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Source_clk_cnt_1_pack_4,
      O => Source_clk_cnt(1)
    );
  Source_n013411 : X_LUT6
    generic map(
      LOC => "SLICE_X5Y34",
      INIT => X"0000F0F00000FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => source_reload_5964,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => Source_done_int_5965,
      O => Source_n0134
    );
  Source_clk_cnt_2 : X_SFF
    generic map(
      LOC => "SLICE_X5Y34",
      INIT => '0'
    )
    port map (
      CE => Source_n0185_inv,
      CLK => clk_pll,
      I => Source_Result(2),
      O => Source_clk_cnt(2),
      SRST => Source_n0134,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_Mcount_clk_cnt_xor_2_11 : X_LUT6
    generic map(
      LOC => "SLICE_X5Y34",
      INIT => X"0FF0FF000FF0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Source_clk_cnt(0),
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_clk_cnt(1),
      ADR5 => '1',
      O => Source_Result(2)
    );
  Source_Mcount_clk_cnt_xor_1_11 : X_LUT5
    generic map(
      LOC => "SLICE_X5Y34",
      INIT => X"0F0FF0F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Source_clk_cnt(0),
      ADR3 => '1',
      ADR4 => Source_clk_cnt(1),
      O => Source_Result(1)
    );
  Source_clk_cnt_1 : X_SFF
    generic map(
      LOC => "SLICE_X5Y34",
      INIT => '0'
    )
    port map (
      CE => Source_n0185_inv,
      CLK => clk_pll,
      I => Source_Result(1),
      O => Source_clk_cnt_1_pack_4,
      SRST => Source_n0134,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_clk_cnt_0 : X_SFF
    generic map(
      LOC => "SLICE_X5Y34",
      INIT => '0'
    )
    port map (
      CE => Source_n0185_inv,
      CLK => clk_pll,
      I => Source_Result(0),
      O => Source_clk_cnt(0),
      SRST => Source_n0134,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_Mcount_clk_cnt_xor_0_11_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X5Y34",
      INIT => X"00FF00FF00FF00FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Source_clk_cnt(0),
      ADR4 => '1',
      ADR5 => '1',
      O => Source_Result(0)
    );
  Source_latched_regs_75 : X_FF
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_75_dpot_2071,
      O => Source_latched_regs(75),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_75_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(74),
      ADR2 => mod_lookup_read_data(75),
      ADR3 => Source_latched_regs(75),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_75_dpot_2071
    );
  Source_latched_regs_74 : X_FF
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_74_dpot_2050,
      O => Source_latched_regs(74),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_74_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(73),
      ADR2 => mod_lookup_read_data(74),
      ADR3 => Source_latched_regs(74),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_74_dpot_2050
    );
  Source_latched_regs_73 : X_FF
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_73_dpot_2057,
      O => Source_latched_regs(73),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_73_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(72),
      ADR2 => mod_lookup_read_data(73),
      ADR3 => Source_latched_regs(73),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_73_dpot_2057
    );
  Source_latched_regs_72 : X_FF
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_72_dpot_2064,
      O => Source_latched_regs(72),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_72_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(71),
      ADR2 => mod_lookup_read_data(72),
      ADR3 => Source_latched_regs(72),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_72_dpot_2064
    );
  SPI_MOD_LOOKUP_DATA_91 : X_FF
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_91_dpot_2091,
      O => SPI_MOD_LOOKUP_DATA(91),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_91_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(75),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(91),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_91_dpot_2091
    );
  SPI_MOD_LOOKUP_DATA_90 : X_FF
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_90_dpot_2100,
      O => SPI_MOD_LOOKUP_DATA(90),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_90_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(74),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(90),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_90_dpot_2100
    );
  SPI_MOD_LOOKUP_DATA_89 : X_FF
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_89_dpot_2107,
      O => SPI_MOD_LOOKUP_DATA(89),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_89_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(73),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(89),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_89_dpot_2107
    );
  SPI_MOD_LOOKUP_DATA_88 : X_FF
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_88_dpot_2084,
      O => SPI_MOD_LOOKUP_DATA(88),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_88_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(72),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(88),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_88_dpot_2084
    );
  SPI_MOD_LOOKUP_DATA_95 : X_FF
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_95_dpot_2121,
      O => SPI_MOD_LOOKUP_DATA(95),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_95_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(79),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(95),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_95_dpot_2121
    );
  SPI_MOD_LOOKUP_DATA_94 : X_FF
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_94_dpot_2130,
      O => SPI_MOD_LOOKUP_DATA(94),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_94_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(78),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(94),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_94_dpot_2130
    );
  SPI_MOD_LOOKUP_DATA_93 : X_FF
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_93_dpot_2137,
      O => SPI_MOD_LOOKUP_DATA(93),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_93_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(77),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(93),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_93_dpot_2137
    );
  SPI_MOD_LOOKUP_DATA_92 : X_FF
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_92_dpot_2114,
      O => SPI_MOD_LOOKUP_DATA(92),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_92_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(76),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(92),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_92_dpot_2114
    );
  Source_latched_regs_115 : X_FF
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_115_dpot_2161,
      O => Source_latched_regs(115),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_115_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(114),
      ADR2 => mod_lookup_read_data(115),
      ADR3 => Source_latched_regs(115),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_115_dpot_2161
    );
  Source_latched_regs_114 : X_FF
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_114_dpot_2140,
      O => Source_latched_regs(114),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_114_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(113),
      ADR2 => mod_lookup_read_data(114),
      ADR3 => Source_latched_regs(114),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot6_0,
      O => Source_latched_regs_114_dpot_2140
    );
  Source_latched_regs_113 : X_FF
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_113_dpot_2147,
      O => Source_latched_regs(113),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_113_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(112),
      ADR2 => mod_lookup_read_data(113),
      ADR3 => Source_latched_regs(113),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_113_dpot_2147
    );
  Source_latched_regs_112 : X_FF
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot3,
      CLK => clk_pll,
      I => Source_latched_regs_112_dpot_2154,
      O => Source_latched_regs(112),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_112_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y44",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(111),
      ADR2 => mod_lookup_read_data(112),
      ADR3 => Source_latched_regs(112),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot5,
      O => Source_latched_regs_112_dpot_2154
    );
  SPI_MOD_LOOKUP_DATA_127 : X_FF
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_127_dpot_2181,
      O => SPI_MOD_LOOKUP_DATA(127),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_127_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(111),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(127),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_127_dpot_2181
    );
  SPI_MOD_LOOKUP_DATA_126 : X_FF
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_126_dpot_2190,
      O => SPI_MOD_LOOKUP_DATA(126),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_126_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(110),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(126),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_126_dpot_2190
    );
  SPI_MOD_LOOKUP_DATA_125 : X_FF
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_125_dpot_2197,
      O => SPI_MOD_LOOKUP_DATA(125),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_125_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(109),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(125),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_125_dpot_2197
    );
  SPI_MOD_LOOKUP_DATA_124 : X_FF
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_124_dpot_2174,
      O => SPI_MOD_LOOKUP_DATA(124),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_124_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X5Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(108),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(124),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_124_dpot_2174
    );
  Modulation_clk_sample_cnt_7_Modulation_clk_sample_cnt_7_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_6_pack_10,
      O => Modulation_clk_sample_cnt(6)
    );
  Modulation_clk_sample_cnt_7_Modulation_clk_sample_cnt_7_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_11_pack_8,
      O => Modulation_clk_sample_cnt(11)
    );
  Modulation_clk_sample_cnt_7_Modulation_clk_sample_cnt_7_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_5_pack_6,
      O => Modulation_clk_sample_cnt(5)
    );
  Modulation_clk_sample_cnt_7_Modulation_clk_sample_cnt_7_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_18_pack_4,
      O => Modulation_clk_sample_cnt(18)
    );
  Modulation_clk_sample_cnt_7 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_7_Q,
      O => Modulation_clk_sample_cnt(7),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux2411 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(7),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_7_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_7_Q
    );
  Modulation_mux2311 : X_LUT5
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_6_0,
      ADR1 => Modulation_clk_sample_cnt(6),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_6_Q
    );
  Modulation_clk_sample_cnt_6 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_6_Q,
      O => Modulation_clk_sample_cnt_6_pack_10,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_4 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_4_Q,
      O => Modulation_clk_sample_cnt(4),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux21111 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(4),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_4_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_4_Q
    );
  Modulation_mux2111 : X_LUT5
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_11_0,
      ADR1 => Modulation_clk_sample_cnt(11),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_11_Q
    );
  Modulation_clk_sample_cnt_11 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_11_Q,
      O => Modulation_clk_sample_cnt_11_pack_8,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_3 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_3_Q,
      O => Modulation_clk_sample_cnt(3),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux2011 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(3),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_3_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_3_Q
    );
  Modulation_mux2211 : X_LUT5
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_5_0,
      ADR1 => Modulation_clk_sample_cnt(5),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_5_Q
    );
  Modulation_clk_sample_cnt_5 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_5_Q,
      O => Modulation_clk_sample_cnt_5_pack_6,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_2 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_2_Q,
      O => Modulation_clk_sample_cnt(2),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux1911 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(2),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_2_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_2_Q
    );
  Modulation_mux911 : X_LUT5
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_18_0,
      ADR1 => Modulation_clk_sample_cnt(18),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_18_Q
    );
  Modulation_clk_sample_cnt_18 : X_SFF
    generic map(
      LOC => "SLICE_X6Y8",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_18_Q,
      O => Modulation_clk_sample_cnt_18_pack_4,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_MOD_LOOKUP_DATA_11 : X_FF
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_11_dpot_2243,
      O => SPI_MOD_LOOKUP_DATA(11),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_11_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(11),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(11),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_11_dpot_2243
    );
  SPI_MOD_LOOKUP_DATA_10 : X_FF
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_10_dpot_2252,
      O => SPI_MOD_LOOKUP_DATA(10),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_10_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(10),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(10),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_10_dpot_2252
    );
  SPI_MOD_LOOKUP_DATA_9 : X_FF
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_9_dpot_2259,
      O => SPI_MOD_LOOKUP_DATA(9),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_9_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(9),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(9),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_9_dpot_2259
    );
  SPI_MOD_LOOKUP_DATA_8 : X_FF
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_8_dpot_2266,
      O => SPI_MOD_LOOKUP_DATA(8),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_8_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y29",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(8),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(8),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_8_dpot_2266
    );
  SPI_MOD_LOOKUP_DATA_7 : X_FF
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_7_dpot_2273,
      O => SPI_MOD_LOOKUP_DATA(7),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_7_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(7),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(7),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_7_dpot_2273
    );
  SPI_MOD_LOOKUP_DATA_6 : X_FF
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_6_dpot_2282,
      O => SPI_MOD_LOOKUP_DATA(6),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_6_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(6),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(6),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_6_dpot_2282
    );
  SPI_MOD_LOOKUP_DATA_5 : X_FF
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_5_dpot_2289,
      O => SPI_MOD_LOOKUP_DATA(5),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_5_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(5),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(5),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_5_dpot_2289
    );
  SPI_MOD_LOOKUP_DATA_4 : X_FF
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_4_dpot_2296,
      O => SPI_MOD_LOOKUP_DATA(4),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_4_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_BUF_OUT(4),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(4),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_4_dpot_2296
    );
  Source_n0127211 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y33",
      INIT => X"0000FF0000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => source_reload_5964,
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => Source_done_int_5965,
      O => Source_n01272
    );
  Source_reg_cnt_0_Source_reg_cnt_0_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Source_reg_cnt_1_pack_9,
      O => Source_reg_cnt(1)
    );
  Source_n01271_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y34",
      INIT => X"FF000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Source_reg_cnt(0),
      ADR4 => Source_reg_cnt(1),
      ADR5 => Source_bit_cnt(5),
      O => N20
    );
  Source_n01271 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y34",
      INIT => X"00AA00AA000000C0"
    )
    port map (
      ADR0 => source_reload_5964,
      ADR1 => Source_latch_5842,
      ADR2 => Source_clk_cnt(2),
      ADR3 => Inst_ResetDelay_OUT_RESET_5868,
      ADR4 => N20,
      ADR5 => Source_done_int_5965,
      O => Source_n0127
    );
  Source_reg_cnt_0 : X_SFF
    generic map(
      LOC => "SLICE_X6Y34",
      INIT => '0'
    )
    port map (
      CE => Source_n0170_inv1_cepot_6743,
      CLK => clk_pll,
      I => Source_reg_cnt_0_dpot_2310,
      O => Source_reg_cnt(0),
      SRST => Source_n01272,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_reg_cnt_0_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y34",
      INIT => X"00FFFF0000FFFF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Source_reg_cnt(0),
      ADR4 => Source_n0170_inv1_rstpot_6820,
      ADR5 => '1',
      O => Source_reg_cnt_0_dpot_2310
    );
  Source_reg_cnt_1_dpot : X_LUT5
    generic map(
      LOC => "SLICE_X6Y34",
      INIT => X"0FF0F0F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Source_reg_cnt(1),
      ADR3 => Source_reg_cnt(0),
      ADR4 => Source_n0170_inv1_rstpot_6820,
      O => Source_reg_cnt_1_dpot_2311
    );
  Source_reg_cnt_1 : X_SFF
    generic map(
      LOC => "SLICE_X6Y34",
      INIT => '0'
    )
    port map (
      CE => Source_n0170_inv1_cepot_6743,
      CLK => clk_pll,
      I => Source_reg_cnt_1_dpot_2311,
      O => Source_reg_cnt_1_pack_9,
      SRST => Source_n01272,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_n0170_inv1_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y34",
      INIT => X"0000C000C000C000"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latch_5842,
      ADR2 => Source_clk_cnt(2),
      ADR3 => Source_bit_cnt(5),
      ADR4 => Source_reg_cnt(0),
      ADR5 => Source_reg_cnt(1),
      O => Source_n0170_inv1_rstpot_6820
    );
  Source_n0181_inv21 : X_LUT6
    generic map(
      LOC => "SLICE_X6Y35",
      INIT => X"000000000000F0C0"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_sclk_5839,
      ADR2 => Source_clk_cnt(2),
      ADR3 => Source_bit_cnt(5),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => Source_done_int_5965,
      O => Source_n0181_inv
    );
  SPI_MOD_LOOKUP_DATA_79 : X_FF
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_79_dpot_2351,
      O => SPI_MOD_LOOKUP_DATA(79),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_79_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_63_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(79),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_79_dpot_2351
    );
  SPI_MOD_LOOKUP_DATA_78 : X_FF
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_78_dpot_2360,
      O => SPI_MOD_LOOKUP_DATA(78),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_78_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_62_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(78),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_78_dpot_2360
    );
  SPI_MOD_LOOKUP_DATA_77 : X_FF
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_77_dpot_2367,
      O => SPI_MOD_LOOKUP_DATA(77),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_77_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_61_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(77),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_77_dpot_2367
    );
  SPI_MOD_LOOKUP_DATA_76 : X_FF
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_76_dpot_2344,
      O => SPI_MOD_LOOKUP_DATA(76),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_76_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_60_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(76),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_76_dpot_2344
    );
  SPI_MOD_LOOKUP_DATA_75 : X_FF
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_75_dpot_2381,
      O => SPI_MOD_LOOKUP_DATA(75),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_75_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(59),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(75),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_75_dpot_2381
    );
  SPI_MOD_LOOKUP_DATA_74 : X_FF
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_74_dpot_2390,
      O => SPI_MOD_LOOKUP_DATA(74),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_74_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(58),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(74),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_74_dpot_2390
    );
  SPI_MOD_LOOKUP_DATA_73 : X_FF
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_73_dpot_2397,
      O => SPI_MOD_LOOKUP_DATA(73),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_73_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(57),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(73),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_73_dpot_2397
    );
  SPI_MOD_LOOKUP_DATA_72 : X_FF
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_72_dpot_2374,
      O => SPI_MOD_LOOKUP_DATA(72),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_72_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(56),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(72),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_72_dpot_2374
    );
  SPI_MOD_LOOKUP_DATA_103 : X_FF
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_103_dpot_2411,
      O => SPI_MOD_LOOKUP_DATA(103),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_103_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(87),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(103),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_103_dpot_2411
    );
  SPI_MOD_LOOKUP_DATA_102 : X_FF
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_102_dpot_2420,
      O => SPI_MOD_LOOKUP_DATA(102),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_102_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(86),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(102),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_102_dpot_2420
    );
  SPI_MOD_LOOKUP_DATA_101 : X_FF
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_101_dpot_2427,
      O => SPI_MOD_LOOKUP_DATA(101),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_101_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(85),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(101),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_101_dpot_2427
    );
  SPI_MOD_LOOKUP_DATA_100 : X_FF
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_100_dpot_2404,
      O => SPI_MOD_LOOKUP_DATA(100),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_100_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(84),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(100),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_100_dpot_2404
    );
  SPI_MOD_LOOKUP_DATA_119 : X_FF
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_119_dpot_2441,
      O => SPI_MOD_LOOKUP_DATA(119),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_119_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_103_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(119),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_119_dpot_2441
    );
  SPI_MOD_LOOKUP_DATA_118 : X_FF
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_118_dpot_2450,
      O => SPI_MOD_LOOKUP_DATA(118),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_118_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_102_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(118),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_118_dpot_2450
    );
  SPI_MOD_LOOKUP_DATA_117 : X_FF
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_117_dpot_2457,
      O => SPI_MOD_LOOKUP_DATA(117),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_117_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_101_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(117),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_117_dpot_2457
    );
  SPI_MOD_LOOKUP_DATA_116 : X_FF
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_116_dpot_2434,
      O => SPI_MOD_LOOKUP_DATA(116),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_116_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y43",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_100_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(116),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_116_dpot_2434
    );
  SPI_MOD_LOOKUP_DATA_111 : X_FF
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_111_dpot_2471,
      O => SPI_MOD_LOOKUP_DATA(111),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_111_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_95_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(111),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_111_dpot_2471
    );
  SPI_MOD_LOOKUP_DATA_110 : X_FF
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_110_dpot_2480,
      O => SPI_MOD_LOOKUP_DATA(110),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_110_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_94_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(110),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_110_dpot_2480
    );
  SPI_MOD_LOOKUP_DATA_109 : X_FF
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_109_dpot_2487,
      O => SPI_MOD_LOOKUP_DATA(109),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_109_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_93_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(109),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_109_dpot_2487
    );
  SPI_MOD_LOOKUP_DATA_108 : X_FF
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_108_dpot_2464,
      O => SPI_MOD_LOOKUP_DATA(108),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_108_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_92_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(108),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_108_dpot_2464
    );
  SPI_MOD_LOOKUP_DATA_123 : X_FF
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_123_dpot_2501,
      O => SPI_MOD_LOOKUP_DATA(123),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_123_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(107),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(123),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_123_dpot_2501
    );
  SPI_MOD_LOOKUP_DATA_122 : X_FF
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_122_dpot_2510,
      O => SPI_MOD_LOOKUP_DATA(122),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_122_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(106),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(122),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_122_dpot_2510
    );
  SPI_MOD_LOOKUP_DATA_121 : X_FF
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_121_dpot_2517,
      O => SPI_MOD_LOOKUP_DATA(121),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_121_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(105),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(121),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_121_dpot_2517
    );
  SPI_MOD_LOOKUP_DATA_120 : X_FF
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_120_dpot_2494,
      O => SPI_MOD_LOOKUP_DATA(120),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_120_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X6Y45",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(104),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(120),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_120_dpot_2494
    );
  SPI_MOD_LOOKUP_DATA_143 : X_FF
    generic map(
      LOC => "SLICE_X6Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_127_0,
      O => SPI_MOD_LOOKUP_DATA(143),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_142 : X_FF
    generic map(
      LOC => "SLICE_X6Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_126_0,
      O => SPI_MOD_LOOKUP_DATA(142),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_141 : X_FF
    generic map(
      LOC => "SLICE_X6Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_125_0,
      O => SPI_MOD_LOOKUP_DATA(141),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_140 : X_FF
    generic map(
      LOC => "SLICE_X6Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_124_0,
      O => SPI_MOD_LOOKUP_DATA(140),
      RST => GND,
      SET => GND
    );
  Modulation_clk_sample_cnt_15_Modulation_clk_sample_cnt_15_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_14_pack_10,
      O => Modulation_clk_sample_cnt(14)
    );
  Modulation_clk_sample_cnt_15_Modulation_clk_sample_cnt_15_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_13_pack_8,
      O => Modulation_clk_sample_cnt(13)
    );
  Modulation_clk_sample_cnt_15_Modulation_clk_sample_cnt_15_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_0_pack_6,
      O => Modulation_clk_sample_cnt(0)
    );
  Modulation_clk_sample_cnt_15_Modulation_clk_sample_cnt_15_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_8_pack_4,
      O => Modulation_clk_sample_cnt(8)
    );
  Modulation_clk_sample_cnt_15 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_15_Q,
      O => Modulation_clk_sample_cnt(15),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux611 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(15),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_15_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_15_Q
    );
  Modulation_mux511 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_14_0,
      ADR1 => Modulation_clk_sample_cnt(14),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_14_Q
    );
  Modulation_clk_sample_cnt_14 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_14_Q,
      O => Modulation_clk_sample_cnt_14_pack_10,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_12 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_12_Q,
      O => Modulation_clk_sample_cnt(12),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux311 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(12),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_12_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_12_Q
    );
  Modulation_mux411 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_13_0,
      ADR1 => Modulation_clk_sample_cnt(13),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_13_Q
    );
  Modulation_clk_sample_cnt_13 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_13_Q,
      O => Modulation_clk_sample_cnt_13_pack_8,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_10 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_10_Q,
      O => Modulation_clk_sample_cnt(10),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux1111 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(10),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_10_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_10_Q
    );
  Modulation_mux1101 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_0_0,
      ADR1 => Modulation_clk_sample_cnt(0),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_0_Q
    );
  Modulation_clk_sample_cnt_0 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_0_Q,
      O => Modulation_clk_sample_cnt_0_pack_6,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_9 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_9_Q,
      O => Modulation_clk_sample_cnt(9),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux2611 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(9),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_9_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_9_Q
    );
  Modulation_mux2511 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_8_0,
      ADR1 => Modulation_clk_sample_cnt(8),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_8_Q
    );
  Modulation_clk_sample_cnt_8 : X_SFF
    generic map(
      LOC => "SLICE_X7Y11",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_8_Q,
      O => Modulation_clk_sample_cnt_8_pack_4,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_MOD_PHASE_INC_11_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y12",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(11),
      ADR1 => SPI_SPI_BUF_OUT(11),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_11_dpot_6454
    );
  Modulation_clk_sample_cnt_23_Modulation_clk_sample_cnt_23_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_22_pack_10,
      O => Modulation_clk_sample_cnt(22)
    );
  Modulation_clk_sample_cnt_23_Modulation_clk_sample_cnt_23_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_19_pack_8,
      O => Modulation_clk_sample_cnt(19)
    );
  Modulation_clk_sample_cnt_23_Modulation_clk_sample_cnt_23_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_1_pack_6,
      O => Modulation_clk_sample_cnt(1)
    );
  Modulation_clk_sample_cnt_23_Modulation_clk_sample_cnt_23_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_16_pack_4,
      O => Modulation_clk_sample_cnt(16)
    );
  Modulation_clk_sample_cnt_23 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_23_Q,
      O => Modulation_clk_sample_cnt(23),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux1511 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(23),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_23_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_23_Q
    );
  Modulation_mux1411 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_22_0,
      ADR1 => Modulation_clk_sample_cnt(22),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_22_Q
    );
  Modulation_clk_sample_cnt_22 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_22_Q,
      O => Modulation_clk_sample_cnt_22_pack_10,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_21 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_21_Q,
      O => Modulation_clk_sample_cnt(21),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux1311 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(21),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_21_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_21_Q
    );
  Modulation_mux1011 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_19_0,
      ADR1 => Modulation_clk_sample_cnt(19),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_19_Q
    );
  Modulation_clk_sample_cnt_19 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_19_Q,
      O => Modulation_clk_sample_cnt_19_pack_8,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_20 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_20_Q,
      O => Modulation_clk_sample_cnt(20),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux1211 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(20),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_20_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_20_Q
    );
  Modulation_mux11111 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_1_0,
      ADR1 => Modulation_clk_sample_cnt(1),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_1_Q
    );
  Modulation_clk_sample_cnt_1 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_1_Q,
      O => Modulation_clk_sample_cnt_1_pack_6,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_17 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_17_Q,
      O => Modulation_clk_sample_cnt(17),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux811 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(17),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_17_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_17_Q
    );
  Modulation_mux711 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_16_0,
      ADR1 => Modulation_clk_sample_cnt(16),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_16_Q
    );
  Modulation_clk_sample_cnt_16 : X_SFF
    generic map(
      LOC => "SLICE_X7Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_16_Q,
      O => Modulation_clk_sample_cnt_16_pack_4,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_clk_sample_cnt_26_Modulation_clk_sample_cnt_26_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_clk_sample_cnt_24_pack_2,
      O => Modulation_clk_sample_cnt(24)
    );
  mod_reset1_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y15",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => SPI_MOD_ENABLE_5927,
      O => mod_reset
    );
  Modulation_clk_sample_cnt_26 : X_SFF
    generic map(
      LOC => "SLICE_X7Y15",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_GND_22_o_MUX_524_o,
      O => Modulation_clk_sample_cnt(26),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_Mmux_clk_sample_cnt_26_GND_22_o_MUX_524_o11 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y15",
      INIT => X"0000000000FF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_clk_sample_cnt(26),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_26_0,
      ADR5 => Sync_AUX3_sync_line(2),
      O => Modulation_clk_sample_cnt_26_GND_22_o_MUX_524_o
    );
  Modulation_clk_sample_cnt_25 : X_SFF
    generic map(
      LOC => "SLICE_X7Y15",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_25_Q,
      O => Modulation_clk_sample_cnt(25),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_mux1711 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y15",
      INIT => X"FF0FF000FF0FF000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => Modulation_clk_sample_cnt(25),
      ADR4 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_25_0,
      ADR5 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_25_Q
    );
  Modulation_mux1611 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y15",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt_26_GND_22_o_add_5_OUT_24_0,
      ADR1 => Modulation_clk_sample_cnt(24),
      ADR2 => Sync_AUX3_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_24_Q
    );
  Modulation_clk_sample_cnt_24 : X_SFF
    generic map(
      LOC => "SLICE_X7Y15",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_clk_sample_cnt_26_clk_sample_cnt_26_mux_6_OUT_24_Q,
      O => Modulation_clk_sample_cnt_24_pack_2,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  source_reload_source_reload_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_NEW_OUTPUT_pack_2,
      O => Modulation_NEW_OUTPUT_6822
    );
  source_reload : X_FF
    generic map(
      LOC => "SLICE_X7Y16",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_NEW_OUTPUT_6822,
      O => source_reload_5964,
      RST => GND,
      SET => GND
    );
  Mmux_LO1_MOSI11 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y16",
      INIT => X"FF000000FF000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Sync_AUX2_sync_line(2),
      ADR4 => MCU_MOSI_IBUF_0,
      ADR5 => '1',
      O => LO1_MOSI_OBUF_5926
    );
  Modulation_NEW_OUTPUT_rstpot : X_LUT5
    generic map(
      LOC => "SLICE_X7Y16",
      INIT => X"E4E4E4E4"
    )
    port map (
      ADR0 => SPI_MOD_ENABLE_5927,
      ADR1 => Modulation_NEW_OUTPUT_6822,
      ADR2 => Modulation_clk_sample_cnt(26),
      ADR3 => '1',
      ADR4 => '1',
      O => Modulation_NEW_OUTPUT_rstpot_2644
    );
  Modulation_NEW_OUTPUT : X_FF
    generic map(
      LOC => "SLICE_X7Y16",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_NEW_OUTPUT_rstpot_2644,
      O => Modulation_NEW_OUTPUT_pack_2,
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_19 : X_FF
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_19_dpot_2663,
      O => SPI_MOD_LOOKUP_DATA(19),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_19_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(3),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(19),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_19_dpot_2663
    );
  SPI_MOD_LOOKUP_DATA_18 : X_FF
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_18_dpot_2672,
      O => SPI_MOD_LOOKUP_DATA(18),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_18_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(2),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(18),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_18_dpot_2672
    );
  SPI_MOD_LOOKUP_DATA_17 : X_FF
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_17_dpot_2679,
      O => SPI_MOD_LOOKUP_DATA(17),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_17_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(1),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(17),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_17_dpot_2679
    );
  SPI_MOD_LOOKUP_DATA_16 : X_FF
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_16_dpot_2656,
      O => SPI_MOD_LOOKUP_DATA(16),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_16_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y30",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(0),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(16),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_16_dpot_2656
    );
  SPI_MOD_LOOKUP_DATA_27 : X_FF
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_27_dpot_2693,
      O => SPI_MOD_LOOKUP_DATA(27),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_27_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(11),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(27),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_27_dpot_2693
    );
  SPI_MOD_LOOKUP_DATA_26 : X_FF
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_26_dpot_2702,
      O => SPI_MOD_LOOKUP_DATA(26),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_26_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(10),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(26),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_26_dpot_2702
    );
  SPI_MOD_LOOKUP_DATA_25 : X_FF
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_25_dpot_2709,
      O => SPI_MOD_LOOKUP_DATA(25),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_25_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(9),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(25),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_25_dpot_2709
    );
  SPI_MOD_LOOKUP_DATA_24 : X_FF
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_24_dpot_2686,
      O => SPI_MOD_LOOKUP_DATA(24),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_24_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(8),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(24),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_24_dpot_2686
    );
  SPI_MOD_LOOKUP_DATA_31 : X_FF
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_31_dpot_2723,
      O => SPI_MOD_LOOKUP_DATA(31),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_31_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(15),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(31),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_31_dpot_2723
    );
  SPI_MOD_LOOKUP_DATA_30 : X_FF
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_30_dpot_2732,
      O => SPI_MOD_LOOKUP_DATA(30),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_30_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(14),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(30),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_30_dpot_2732
    );
  SPI_MOD_LOOKUP_DATA_29 : X_FF
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_29_dpot_2739,
      O => SPI_MOD_LOOKUP_DATA(29),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_29_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(13),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(29),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_29_dpot_2739
    );
  SPI_MOD_LOOKUP_DATA_28 : X_FF
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_28_dpot_2716,
      O => SPI_MOD_LOOKUP_DATA(28),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_28_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(12),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(28),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_28_dpot_2716
    );
  SPI_mod_lookup_buffer_15_SPI_mod_lookup_buffer_15_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(31),
      O => SPI_mod_lookup_buffer_31_0
    );
  SPI_mod_lookup_buffer_15_SPI_mod_lookup_buffer_15_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(30),
      O => SPI_mod_lookup_buffer_30_0
    );
  SPI_mod_lookup_buffer_15_SPI_mod_lookup_buffer_15_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(29),
      O => SPI_mod_lookup_buffer_29_0
    );
  SPI_mod_lookup_buffer_15_SPI_mod_lookup_buffer_15_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(28),
      O => SPI_mod_lookup_buffer_28_0
    );
  SPI_mod_lookup_buffer_15 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(15),
      O => SPI_mod_lookup_buffer(15),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_15_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(15),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_15_rt_2744
    );
  SPI_mod_lookup_buffer_31 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_15_rt_2744,
      O => SPI_mod_lookup_buffer(31),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_14 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(14),
      O => SPI_mod_lookup_buffer(14),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_14_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(14),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_14_rt_2751
    );
  SPI_mod_lookup_buffer_30 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_14_rt_2751,
      O => SPI_mod_lookup_buffer(30),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_13 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(13),
      O => SPI_mod_lookup_buffer(13),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_13_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(13),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_13_rt_2756
    );
  SPI_mod_lookup_buffer_29 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_13_rt_2756,
      O => SPI_mod_lookup_buffer(29),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_12 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(12),
      O => SPI_mod_lookup_buffer(12),
      RST => GND,
      SET => GND
    );
  Source_n0170_inv1_cepot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => X"00000F0F00000F0F"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Source_done_int_1_6733,
      ADR3 => '1',
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => '1',
      O => Source_n0170_inv1_cepot_6743
    );
  SPI_mod_lookup_buffer_12_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(12),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_12_rt_2762
    );
  SPI_mod_lookup_buffer_28 : X_FF
    generic map(
      LOC => "SLICE_X7Y33",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_12_rt_2762,
      O => SPI_mod_lookup_buffer(28),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_3 : X_FF
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_ATTENUATION_3_dpot_2775,
      O => SPI_SOURCE_ATTENUATION(3),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_3_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_ATTENUATION(3),
      ADR4 => SPI_SPI_BUF_OUT(8),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_ATTENUATION_3_dpot_2775
    );
  SPI_SOURCE_ATTENUATION_2 : X_FF
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_ATTENUATION_2_dpot_2782,
      O => SPI_SOURCE_ATTENUATION(2),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_2_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_ATTENUATION(2),
      ADR4 => SPI_SPI_BUF_OUT(7),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_ATTENUATION_2_dpot_2782
    );
  SPI_SOURCE_ATTENUATION_1 : X_FF
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_ATTENUATION_1_dpot_2787,
      O => SPI_SOURCE_ATTENUATION(1),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_1_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_ATTENUATION(1),
      ADR4 => SPI_SPI_BUF_OUT(6),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_ATTENUATION_1_dpot_2787
    );
  SPI_SOURCE_ATTENUATION_0 : X_FF
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_ATTENUATION_0_dpot_2770,
      O => SPI_SOURCE_ATTENUATION(0),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_0_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y34",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_ATTENUATION(0),
      ADR4 => SPI_SPI_BUF_OUT(5),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_ATTENUATION_0_dpot_2770
    );
  Source_bit_cnt_5_Source_bit_cnt_5_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Source_bit_cnt_3_pack_17,
      O => Source_bit_cnt(3)
    );
  Source_bit_cnt_5_Source_bit_cnt_5_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Source_bit_cnt_0_pack_15,
      O => Source_bit_cnt(0)
    );
  Source_bit_cnt_5 : X_SFF
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => '0'
    )
    port map (
      CE => Source_n0181_inv,
      CLK => clk_pll,
      I => Source_Mcount_bit_cnt5,
      O => Source_bit_cnt(5),
      SRST => Source_n0127,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_Mcount_bit_cnt_xor_5_11 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => X"FF80FF00FF00FF00"
    )
    port map (
      ADR0 => Source_bit_cnt(4),
      ADR1 => Source_bit_cnt(3),
      ADR2 => Source_bit_cnt(2),
      ADR3 => Source_bit_cnt(5),
      ADR4 => Source_bit_cnt(1),
      ADR5 => Source_bit_cnt(0),
      O => Source_Mcount_bit_cnt5
    );
  Source_bit_cnt_4 : X_SFF
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => '0'
    )
    port map (
      CE => Source_n0181_inv,
      CLK => clk_pll,
      I => Source_Mcount_bit_cnt4,
      O => Source_bit_cnt(4),
      SRST => Source_n0127,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_Mcount_bit_cnt41 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => X"FF00FF007F80FF00"
    )
    port map (
      ADR0 => Source_bit_cnt(2),
      ADR1 => Source_bit_cnt(3),
      ADR2 => Source_bit_cnt(0),
      ADR3 => Source_bit_cnt(4),
      ADR4 => Source_bit_cnt(1),
      ADR5 => Source_bit_cnt(5),
      O => Source_Mcount_bit_cnt4
    );
  Source_bit_cnt_2 : X_SFF
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => '0'
    )
    port map (
      CE => Source_n0181_inv,
      CLK => clk_pll,
      I => Source_Mcount_bit_cnt2,
      O => Source_bit_cnt(2),
      SRST => Source_n0127,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_Mcount_bit_cnt21 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => X"FF003FC0FF003FC0"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_bit_cnt(0),
      ADR2 => Source_bit_cnt(1),
      ADR3 => Source_bit_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => '1',
      O => Source_Mcount_bit_cnt2
    );
  Source_Mcount_bit_cnt31 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => X"AAAA6AAA"
    )
    port map (
      ADR0 => Source_bit_cnt(3),
      ADR1 => Source_bit_cnt(0),
      ADR2 => Source_bit_cnt(1),
      ADR3 => Source_bit_cnt(2),
      ADR4 => Source_bit_cnt(5),
      O => Source_Mcount_bit_cnt3
    );
  Source_bit_cnt_3 : X_SFF
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => '0'
    )
    port map (
      CE => Source_n0181_inv,
      CLK => clk_pll,
      I => Source_Mcount_bit_cnt3,
      O => Source_bit_cnt_3_pack_17,
      SRST => Source_n0127,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_bit_cnt_1 : X_SFF
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => '0'
    )
    port map (
      CE => Source_n0181_inv,
      CLK => clk_pll,
      I => Source_Mcount_bit_cnt1,
      O => Source_bit_cnt(1),
      SRST => Source_n0127,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_Mcount_bit_cnt_xor_1_11 : X_LUT6
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => X"F00FFF00F00FFF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Source_bit_cnt(5),
      ADR3 => Source_bit_cnt(1),
      ADR4 => Source_bit_cnt(0),
      ADR5 => '1',
      O => Source_Mcount_bit_cnt1
    );
  Source_Mcount_bit_cnt_xor_0_11 : X_LUT5
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => X"F0F00F0F"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Source_bit_cnt(5),
      ADR3 => '1',
      ADR4 => Source_bit_cnt(0),
      O => Source_Mcount_bit_cnt
    );
  Source_bit_cnt_0 : X_SFF
    generic map(
      LOC => "SLICE_X7Y35",
      INIT => '0'
    )
    port map (
      CE => Source_n0181_inv,
      CLK => clk_pll,
      I => Source_Mcount_bit_cnt,
      O => Source_bit_cnt_0_pack_15,
      SRST => Source_n0127,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_MOD_LOOKUP_INDEX_3_SPI_MOD_LOOKUP_INDEX_3_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_INDEX(7),
      O => SPI_MOD_LOOKUP_INDEX_7_0
    );
  SPI_MOD_LOOKUP_INDEX_3_SPI_MOD_LOOKUP_INDEX_3_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_INDEX(6),
      O => SPI_MOD_LOOKUP_INDEX_6_0
    );
  SPI_MOD_LOOKUP_INDEX_3_SPI_MOD_LOOKUP_INDEX_3_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_INDEX(5),
      O => SPI_MOD_LOOKUP_INDEX_5_0
    );
  SPI_MOD_LOOKUP_INDEX_3_SPI_MOD_LOOKUP_INDEX_3_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_INDEX(4),
      O => SPI_MOD_LOOKUP_INDEX_4_0
    );
  SPI_MOD_LOOKUP_INDEX_3 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(3),
      O => SPI_MOD_LOOKUP_INDEX(3),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_7_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_BUF_OUT(7),
      O => SPI_SPI_BUF_OUT_7_rt_2829
    );
  SPI_MOD_LOOKUP_INDEX_7 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT_7_rt_2829,
      O => SPI_MOD_LOOKUP_INDEX(7),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_INDEX_2 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(2),
      O => SPI_MOD_LOOKUP_INDEX(2),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_6_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_BUF_OUT(6),
      O => SPI_SPI_BUF_OUT_6_rt_2836
    );
  SPI_MOD_LOOKUP_INDEX_6 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT_6_rt_2836,
      O => SPI_MOD_LOOKUP_INDEX(6),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_INDEX_1 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(1),
      O => SPI_MOD_LOOKUP_INDEX(1),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_5_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_BUF_OUT(5),
      O => SPI_SPI_BUF_OUT_5_rt_2841
    );
  SPI_MOD_LOOKUP_INDEX_5 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT_5_rt_2841,
      O => SPI_MOD_LOOKUP_INDEX(5),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_INDEX_0 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(0),
      O => SPI_MOD_LOOKUP_INDEX(0),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_4_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_BUF_OUT(4),
      O => SPI_SPI_BUF_OUT_4_rt_2846
    );
  SPI_MOD_LOOKUP_INDEX_4 : X_FF
    generic map(
      LOC => "SLICE_X7Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0453_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT_4_rt_2846,
      O => SPI_MOD_LOOKUP_INDEX(4),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_83 : X_FF
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_83_dpot_2860,
      O => SPI_MOD_LOOKUP_DATA(83),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_83_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_67_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(83),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_83_dpot_2860
    );
  SPI_MOD_LOOKUP_DATA_82 : X_FF
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_82_dpot_2869,
      O => SPI_MOD_LOOKUP_DATA(82),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_82_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_66_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(82),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_82_dpot_2869
    );
  SPI_MOD_LOOKUP_DATA_81 : X_FF
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_81_dpot_2876,
      O => SPI_MOD_LOOKUP_DATA(81),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_81_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_65_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(81),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_81_dpot_2876
    );
  SPI_MOD_LOOKUP_DATA_80 : X_FF
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_80_dpot_2853,
      O => SPI_MOD_LOOKUP_DATA(80),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_80_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X7Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_64_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(80),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_80_dpot_2853
    );
  SPI_mod_lookup_buffer_75_SPI_mod_lookup_buffer_75_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(91),
      O => SPI_mod_lookup_buffer_91_0
    );
  SPI_mod_lookup_buffer_75_SPI_mod_lookup_buffer_75_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(90),
      O => SPI_mod_lookup_buffer_90_0
    );
  SPI_mod_lookup_buffer_75_SPI_mod_lookup_buffer_75_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(89),
      O => SPI_mod_lookup_buffer_89_0
    );
  SPI_mod_lookup_buffer_75_SPI_mod_lookup_buffer_75_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(88),
      O => SPI_mod_lookup_buffer_88_0
    );
  SPI_mod_lookup_buffer_75 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(59),
      O => SPI_mod_lookup_buffer(75),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_75_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(75),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_75_rt_2889
    );
  SPI_mod_lookup_buffer_91 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_75_rt_2889,
      O => SPI_mod_lookup_buffer(91),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_74 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(58),
      O => SPI_mod_lookup_buffer(74),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_74_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(74),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_74_rt_2896
    );
  SPI_mod_lookup_buffer_90 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_74_rt_2896,
      O => SPI_mod_lookup_buffer(90),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_73 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(57),
      O => SPI_mod_lookup_buffer(73),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_73_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(73),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_73_rt_2879
    );
  SPI_mod_lookup_buffer_89 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_73_rt_2879,
      O => SPI_mod_lookup_buffer(89),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_72 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(56),
      O => SPI_mod_lookup_buffer(72),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_72_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(72),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_72_rt_2884
    );
  SPI_mod_lookup_buffer_88 : X_FF
    generic map(
      LOC => "SLICE_X7Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_72_rt_2884,
      O => SPI_mod_lookup_buffer(88),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_79_SPI_mod_lookup_buffer_79_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(95),
      O => SPI_mod_lookup_buffer_95_0
    );
  SPI_mod_lookup_buffer_79_SPI_mod_lookup_buffer_79_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(94),
      O => SPI_mod_lookup_buffer_94_0
    );
  SPI_mod_lookup_buffer_79_SPI_mod_lookup_buffer_79_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(93),
      O => SPI_mod_lookup_buffer_93_0
    );
  SPI_mod_lookup_buffer_79_SPI_mod_lookup_buffer_79_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(92),
      O => SPI_mod_lookup_buffer_92_0
    );
  SPI_mod_lookup_buffer_79 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_63_0,
      O => SPI_mod_lookup_buffer(79),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_79_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(79),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_79_rt_2911
    );
  SPI_mod_lookup_buffer_95 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_79_rt_2911,
      O => SPI_mod_lookup_buffer(95),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_78 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_62_0,
      O => SPI_mod_lookup_buffer(78),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_78_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(78),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_78_rt_2918
    );
  SPI_mod_lookup_buffer_94 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_78_rt_2918,
      O => SPI_mod_lookup_buffer(94),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_77 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_61_0,
      O => SPI_mod_lookup_buffer(77),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_77_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(77),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_77_rt_2901
    );
  SPI_mod_lookup_buffer_93 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_77_rt_2901,
      O => SPI_mod_lookup_buffer(93),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_76 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_60_0,
      O => SPI_mod_lookup_buffer(76),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_76_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(76),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_76_rt_2906
    );
  SPI_mod_lookup_buffer_92 : X_FF
    generic map(
      LOC => "SLICE_X7Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_76_rt_2906,
      O => SPI_mod_lookup_buffer(92),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_111_SPI_mod_lookup_buffer_111_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(127),
      O => SPI_mod_lookup_buffer_127_0
    );
  SPI_mod_lookup_buffer_111_SPI_mod_lookup_buffer_111_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(126),
      O => SPI_mod_lookup_buffer_126_0
    );
  SPI_mod_lookup_buffer_111_SPI_mod_lookup_buffer_111_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(125),
      O => SPI_mod_lookup_buffer_125_0
    );
  SPI_mod_lookup_buffer_111_SPI_mod_lookup_buffer_111_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(124),
      O => SPI_mod_lookup_buffer_124_0
    );
  SPI_mod_lookup_buffer_111 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_95_0,
      O => SPI_mod_lookup_buffer(111),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_111_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(111),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_111_rt_2933
    );
  SPI_mod_lookup_buffer_127 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_111_rt_2933,
      O => SPI_mod_lookup_buffer(127),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_110 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_94_0,
      O => SPI_mod_lookup_buffer(110),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_110_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(110),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_110_rt_2940
    );
  SPI_mod_lookup_buffer_126 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_110_rt_2940,
      O => SPI_mod_lookup_buffer(126),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_109 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_93_0,
      O => SPI_mod_lookup_buffer(109),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_109_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(109),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_109_rt_2923
    );
  SPI_mod_lookup_buffer_125 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_109_rt_2923,
      O => SPI_mod_lookup_buffer(125),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_108 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_92_0,
      O => SPI_mod_lookup_buffer(108),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_108_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(108),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_108_rt_2928
    );
  SPI_mod_lookup_buffer_124 : X_FF
    generic map(
      LOC => "SLICE_X7Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_108_rt_2928,
      O => SPI_mod_lookup_buffer(124),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_107_SPI_mod_lookup_buffer_107_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(123),
      O => SPI_mod_lookup_buffer_123_0
    );
  SPI_mod_lookup_buffer_107_SPI_mod_lookup_buffer_107_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(122),
      O => SPI_mod_lookup_buffer_122_0
    );
  SPI_mod_lookup_buffer_107_SPI_mod_lookup_buffer_107_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(121),
      O => SPI_mod_lookup_buffer_121_0
    );
  SPI_mod_lookup_buffer_107_SPI_mod_lookup_buffer_107_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(120),
      O => SPI_mod_lookup_buffer_120_0
    );
  SPI_mod_lookup_buffer_107 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_91_0,
      O => SPI_mod_lookup_buffer(107),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_107_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(107),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_107_rt_2955
    );
  SPI_mod_lookup_buffer_123 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_107_rt_2955,
      O => SPI_mod_lookup_buffer(123),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_106 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_90_0,
      O => SPI_mod_lookup_buffer(106),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_106_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(106),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_106_rt_2962
    );
  SPI_mod_lookup_buffer_122 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_106_rt_2962,
      O => SPI_mod_lookup_buffer(122),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_105 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_89_0,
      O => SPI_mod_lookup_buffer(105),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_105_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(105),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_105_rt_2945
    );
  SPI_mod_lookup_buffer_121 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_105_rt_2945,
      O => SPI_mod_lookup_buffer(121),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_104 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_88_0,
      O => SPI_mod_lookup_buffer(104),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_104_rt : X_LUT5
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(104),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_104_rt_2950
    );
  SPI_mod_lookup_buffer_120 : X_FF
    generic map(
      LOC => "SLICE_X7Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_104_rt_2950,
      O => SPI_mod_lookup_buffer(120),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_139 : X_FF
    generic map(
      LOC => "SLICE_X7Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_123_0,
      O => SPI_MOD_LOOKUP_DATA(139),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_138 : X_FF
    generic map(
      LOC => "SLICE_X7Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_122_0,
      O => SPI_MOD_LOOKUP_DATA(138),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_137 : X_FF
    generic map(
      LOC => "SLICE_X7Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_121_0,
      O => SPI_MOD_LOOKUP_DATA(137),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_136 : X_FF
    generic map(
      LOC => "SLICE_X7Y46",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_120_0,
      O => SPI_MOD_LOOKUP_DATA(136),
      RST => GND,
      SET => GND
    );
  SOURCE_LE_OBUF_SOURCE_LE_OBUF_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SOURCE_CLK_OBUF_2978,
      O => SOURCE_CLK_OBUF_0
    );
  Mmux_SOURCE_LE11 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y10",
      INIT => X"FFF00F00FFF00F00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX1_sync_line(2),
      ADR3 => Source_latch_5842,
      ADR4 => MCU_NSS_IBUF_0,
      ADR5 => '1',
      O => SOURCE_LE_OBUF_5838
    );
  Mmux_SOURCE_CLK11 : X_LUT5
    generic map(
      LOC => "SLICE_X8Y10",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Source_sclk_5839,
      ADR1 => MCU_SCK_IBUF_0,
      ADR2 => Sync_AUX1_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => SOURCE_CLK_OBUF_2978
    );
  SPI_MOD_PHASE_INC_15_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y12",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(15),
      ADR1 => SPI_SPI_BUF_OUT(15),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_15_dpot_6828
    );
  SPI_MOD_PHASE_INC_15 : X_FF
    generic map(
      LOC => "SLICE_X8Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_15_dpot1_2995,
      O => SPI_MOD_PHASE_INC(15),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_15_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y12",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(15),
      ADR4 => SPI_MOD_PHASE_INC_15_dpot_6828,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_15_dpot1_2995
    );
  SPI_MOD_PHASE_INC_6 : X_FF
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_6_dpot1_3006,
      O => SPI_MOD_PHASE_INC(6),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_6_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(6),
      ADR4 => SPI_MOD_PHASE_INC_6_dpot_6539,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_6_dpot1_3006
    );
  SPI_MOD_PHASE_INC_5 : X_FF
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_5_dpot1_3013,
      O => SPI_MOD_PHASE_INC(5),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_5_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(5),
      ADR4 => SPI_MOD_PHASE_INC_5_dpot_6538,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_5_dpot1_3013
    );
  SPI_MOD_PHASE_INC_4 : X_FF
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_4_dpot1_3018,
      O => SPI_MOD_PHASE_INC(4),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_4_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(4),
      ADR4 => SPI_MOD_PHASE_INC_4_dpot_6537,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_4_dpot1_3018
    );
  SPI_MOD_PHASE_INC_3 : X_FF
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_3_dpot1_3001,
      O => SPI_MOD_PHASE_INC(3),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_3_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y15",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(3),
      ADR4 => SPI_MOD_PHASE_INC_3_dpot_6536,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_3_dpot1_3001
    );
  SPI_MOD_ENABLE : X_SFF
    generic map(
      LOC => "SLICE_X8Y16",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_MOD_ENABLE_rstpot_3023,
      O => SPI_MOD_ENABLE_5927,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_MOD_ENABLE_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y16",
      INIT => X"FF14EB00FF00FF00"
    )
    port map (
      ADR0 => SPI_selected_register(2),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_MOD_ENABLE_5927,
      ADR4 => SPI_n0621,
      ADR5 => SPI_n0539_inv111,
      O => SPI_MOD_ENABLE_rstpot_3023
    );
  SPI_n0539_inv11_2 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y16",
      INIT => X"0000000010000000"
    )
    port map (
      ADR0 => SPI_selected_register(4),
      ADR1 => SPI_selected_register(3),
      ADR2 => SPI_SPI_COMPLETE_6402,
      ADR3 => SPI_state_FSM_FFd2_6400,
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_NSS_last_NSS_AND_33_o,
      O => SPI_n0539_inv111
    );
  SPI_MOD_PHASE_INC_7_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y21",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(7),
      ADR1 => SPI_SPI_BUF_OUT(7),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_7_dpot_6540
    );
  Modulation_OUTPUT_SAMPLE_3_Modulation_OUTPUT_SAMPLE_3_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_OUTPUT_SAMPLE(7),
      O => Modulation_OUTPUT_SAMPLE_7_0
    );
  Modulation_OUTPUT_SAMPLE_3_Modulation_OUTPUT_SAMPLE_3_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_OUTPUT_SAMPLE(6),
      O => Modulation_OUTPUT_SAMPLE_6_0
    );
  Modulation_OUTPUT_SAMPLE_3_Modulation_OUTPUT_SAMPLE_3_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_OUTPUT_SAMPLE(5),
      O => Modulation_OUTPUT_SAMPLE_5_0
    );
  Modulation_OUTPUT_SAMPLE_3_Modulation_OUTPUT_SAMPLE_3_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_OUTPUT_SAMPLE(4),
      O => Modulation_OUTPUT_SAMPLE_4_0
    );
  Modulation_OUTPUT_SAMPLE_3 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample(3),
      O => Modulation_OUTPUT_SAMPLE(3),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_sample_7_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_sample(7),
      O => Modulation_sample_7_rt_3057
    );
  Modulation_OUTPUT_SAMPLE_7 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample_7_rt_3057,
      O => Modulation_OUTPUT_SAMPLE(7),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_OUTPUT_SAMPLE_2 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample(2),
      O => Modulation_OUTPUT_SAMPLE(2),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_sample_6_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_sample(6),
      O => Modulation_sample_6_rt_3064
    );
  Modulation_OUTPUT_SAMPLE_6 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample_6_rt_3064,
      O => Modulation_OUTPUT_SAMPLE(6),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_OUTPUT_SAMPLE_1 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample(1),
      O => Modulation_OUTPUT_SAMPLE(1),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_sample_5_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_sample(5),
      O => Modulation_sample_5_rt_3046
    );
  Modulation_OUTPUT_SAMPLE_5 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample_5_rt_3046,
      O => Modulation_OUTPUT_SAMPLE(5),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_OUTPUT_SAMPLE_0 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample(0),
      O => Modulation_OUTPUT_SAMPLE(0),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_sample_4_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_sample(4),
      O => Modulation_sample_4_rt_3051
    );
  Modulation_OUTPUT_SAMPLE_4 : X_SFF
    generic map(
      LOC => "SLICE_X8Y28",
      INIT => '0'
    )
    port map (
      CE => Modulation_clk_sample_cnt(26),
      CLK => clk_pll,
      I => Modulation_sample_4_rt_3051,
      O => Modulation_OUTPUT_SAMPLE(4),
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_n0419_13_11 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y29",
      INIT => X"00000000FFFD0000"
    )
    port map (
      ADR0 => SPI_last_NSS_5865,
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => Sync_AUX1_sync_line(2),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_n0419_13_1
    );
  SPI_MOD_LOOKUP_DATA_35 : X_FF
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_35_dpot_3086,
      O => SPI_MOD_LOOKUP_DATA(35),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_35_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_19_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(35),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_35_dpot_3086
    );
  SPI_MOD_LOOKUP_DATA_34 : X_FF
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_34_dpot_3095,
      O => SPI_MOD_LOOKUP_DATA(34),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_34_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_18_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(34),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_34_dpot_3095
    );
  SPI_MOD_LOOKUP_DATA_33 : X_FF
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_33_dpot_3102,
      O => SPI_MOD_LOOKUP_DATA(33),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_33_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_17_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(33),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_33_dpot_3102
    );
  SPI_MOD_LOOKUP_DATA_32 : X_FF
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_32_dpot_3079,
      O => SPI_MOD_LOOKUP_DATA(32),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_32_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y31",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_16_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(32),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_32_dpot_3079
    );
  SPI_SOURCE_PORTSELECT : X_FF
    generic map(
      LOC => "SLICE_X8Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_PORTSELECT_dpot_3109,
      O => SPI_SOURCE_PORTSELECT_5829,
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_PORTSELECT_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y32",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_PORTSELECT_5829,
      ADR4 => SPI_SPI_BUF_OUT(3),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_PORTSELECT_dpot_3109
    );
  SPI_mod_lookup_buffer_11_SPI_mod_lookup_buffer_11_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(27),
      O => SPI_mod_lookup_buffer_27_0
    );
  SPI_mod_lookup_buffer_11_SPI_mod_lookup_buffer_11_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(26),
      O => SPI_mod_lookup_buffer_26_0
    );
  SPI_mod_lookup_buffer_11_SPI_mod_lookup_buffer_11_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(25),
      O => SPI_mod_lookup_buffer_25_0
    );
  SPI_mod_lookup_buffer_11_SPI_mod_lookup_buffer_11_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(24),
      O => SPI_mod_lookup_buffer_24_0
    );
  SPI_mod_lookup_buffer_11 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(11),
      O => SPI_mod_lookup_buffer(11),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_11_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(11),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_11_rt_3122
    );
  SPI_mod_lookup_buffer_27 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_11_rt_3122,
      O => SPI_mod_lookup_buffer(27),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_10 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(10),
      O => SPI_mod_lookup_buffer(10),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_10_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(10),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_10_rt_3129
    );
  SPI_mod_lookup_buffer_26 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_10_rt_3129,
      O => SPI_mod_lookup_buffer(26),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_9 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(9),
      O => SPI_mod_lookup_buffer(9),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_9_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(9),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_9_rt_3112
    );
  SPI_mod_lookup_buffer_25 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_9_rt_3112,
      O => SPI_mod_lookup_buffer(25),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_8 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(8),
      O => SPI_mod_lookup_buffer(8),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_8_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(8),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_8_rt_3117
    );
  SPI_mod_lookup_buffer_24 : X_FF
    generic map(
      LOC => "SLICE_X8Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_8_rt_3117,
      O => SPI_mod_lookup_buffer(24),
      RST => GND,
      SET => GND
    );
  SPI_n0419_13_11_4 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y35",
      INIT => X"00000000FFFD0000"
    )
    port map (
      ADR0 => SPI_last_NSS_5865,
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => Sync_AUX1_sync_line(2),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_n0419_13_113
    );
  SPI_mod_lookup_buffer_71_SPI_mod_lookup_buffer_71_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(67),
      O => SPI_mod_lookup_buffer_67_0
    );
  SPI_mod_lookup_buffer_71_SPI_mod_lookup_buffer_71_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(66),
      O => SPI_mod_lookup_buffer_66_0
    );
  SPI_mod_lookup_buffer_71_SPI_mod_lookup_buffer_71_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(65),
      O => SPI_mod_lookup_buffer_65_0
    );
  SPI_mod_lookup_buffer_71_SPI_mod_lookup_buffer_71_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(64),
      O => SPI_mod_lookup_buffer_64_0
    );
  SPI_mod_lookup_buffer_71 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_55_0,
      O => SPI_mod_lookup_buffer(71),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_51_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(51),
      O => SPI_mod_lookup_buffer_51_rt_3143
    );
  SPI_mod_lookup_buffer_67 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_51_rt_3143,
      O => SPI_mod_lookup_buffer(67),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_70 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_54_0,
      O => SPI_mod_lookup_buffer(70),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_50_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(50),
      O => SPI_mod_lookup_buffer_50_rt_3150
    );
  SPI_mod_lookup_buffer_66 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_50_rt_3150,
      O => SPI_mod_lookup_buffer(66),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_69 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_53_0,
      O => SPI_mod_lookup_buffer(69),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_49_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(49),
      O => SPI_mod_lookup_buffer_49_rt_3155
    );
  SPI_mod_lookup_buffer_65 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_49_rt_3155,
      O => SPI_mod_lookup_buffer(65),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_68 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_52_0,
      O => SPI_mod_lookup_buffer(68),
      RST => GND,
      SET => GND
    );
  SPI_n0578_inv1_2 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => X"0030000000300000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_6694,
      ADR2 => SPI_state_FSM_FFd2_1_6695,
      ADR3 => SPI_state_FSM_FFd1_1_6696,
      ADR4 => SPI_n0419_13_11_6691,
      ADR5 => '1',
      O => SPI_n0578_inv11
    );
  SPI_mod_lookup_buffer_48_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => X"AAAAAAAA"
    )
    port map (
      ADR0 => SPI_mod_lookup_buffer(48),
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_48_rt_3161
    );
  SPI_mod_lookup_buffer_64 : X_FF
    generic map(
      LOC => "SLICE_X8Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_48_rt_3161,
      O => SPI_mod_lookup_buffer(64),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_87 : X_FF
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_87_dpot_3180,
      O => SPI_MOD_LOOKUP_DATA(87),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_87_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(71),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(87),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_87_dpot_3180
    );
  SPI_MOD_LOOKUP_DATA_86 : X_FF
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_86_dpot_3189,
      O => SPI_MOD_LOOKUP_DATA(86),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_86_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(70),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(86),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_86_dpot_3189
    );
  SPI_MOD_LOOKUP_DATA_85 : X_FF
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_85_dpot_3196,
      O => SPI_MOD_LOOKUP_DATA(85),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_85_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(69),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(85),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_85_dpot_3196
    );
  SPI_MOD_LOOKUP_DATA_84 : X_FF
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_84_dpot_3173,
      O => SPI_MOD_LOOKUP_DATA(84),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_84_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y39",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(68),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(84),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_84_dpot_3173
    );
  SPI_MOD_LOOKUP_DATA_99 : X_FF
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_99_dpot_3210,
      O => SPI_MOD_LOOKUP_DATA(99),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_99_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(83),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(99),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_99_dpot_3210
    );
  SPI_MOD_LOOKUP_DATA_98 : X_FF
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_98_dpot_3219,
      O => SPI_MOD_LOOKUP_DATA(98),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_98_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(82),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(98),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_98_dpot_3219
    );
  SPI_MOD_LOOKUP_DATA_97 : X_FF
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_97_dpot_3226,
      O => SPI_MOD_LOOKUP_DATA(97),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_97_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(81),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(97),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_97_dpot_3226
    );
  SPI_MOD_LOOKUP_DATA_96 : X_FF
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_96_dpot_3203,
      O => SPI_MOD_LOOKUP_DATA(96),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_96_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y40",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(80),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(96),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_96_dpot_3203
    );
  SPI_mod_lookup_buffer_87 : X_FF
    generic map(
      LOC => "SLICE_X8Y41",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(71),
      O => SPI_mod_lookup_buffer(87),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_86 : X_FF
    generic map(
      LOC => "SLICE_X8Y41",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(70),
      O => SPI_mod_lookup_buffer(86),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_85 : X_FF
    generic map(
      LOC => "SLICE_X8Y41",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(69),
      O => SPI_mod_lookup_buffer(85),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_84 : X_FF
    generic map(
      LOC => "SLICE_X8Y41",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(68),
      O => SPI_mod_lookup_buffer(84),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_99_SPI_mod_lookup_buffer_99_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(103),
      O => SPI_mod_lookup_buffer_103_0
    );
  SPI_mod_lookup_buffer_99_SPI_mod_lookup_buffer_99_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(102),
      O => SPI_mod_lookup_buffer_102_0
    );
  SPI_mod_lookup_buffer_99_SPI_mod_lookup_buffer_99_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(101),
      O => SPI_mod_lookup_buffer_101_0
    );
  SPI_mod_lookup_buffer_99_SPI_mod_lookup_buffer_99_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(100),
      O => SPI_mod_lookup_buffer_100_0
    );
  SPI_mod_lookup_buffer_99 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(83),
      O => SPI_mod_lookup_buffer(99),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_87_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(87),
      O => SPI_mod_lookup_buffer_87_rt_3241
    );
  SPI_mod_lookup_buffer_103 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_87_rt_3241,
      O => SPI_mod_lookup_buffer(103),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_98 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(82),
      O => SPI_mod_lookup_buffer(98),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_86_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(86),
      O => SPI_mod_lookup_buffer_86_rt_3248
    );
  SPI_mod_lookup_buffer_102 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_86_rt_3248,
      O => SPI_mod_lookup_buffer(102),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_97 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(81),
      O => SPI_mod_lookup_buffer(97),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_85_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(85),
      O => SPI_mod_lookup_buffer_85_rt_3253
    );
  SPI_mod_lookup_buffer_101 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_85_rt_3253,
      O => SPI_mod_lookup_buffer(101),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_96 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(80),
      O => SPI_mod_lookup_buffer(96),
      RST => GND,
      SET => GND
    );
  SPI_n0578_inv1_3 : X_LUT6
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => X"0030000000300000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_6694,
      ADR2 => SPI_state_FSM_FFd2_1_6695,
      ADR3 => SPI_state_FSM_FFd1_1_6696,
      ADR4 => SPI_n0419_13_11_6691,
      ADR5 => '1',
      O => SPI_n0578_inv12
    );
  SPI_mod_lookup_buffer_84_rt : X_LUT5
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => X"AAAAAAAA"
    )
    port map (
      ADR0 => SPI_mod_lookup_buffer(84),
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_84_rt_3259
    );
  SPI_mod_lookup_buffer_100 : X_FF
    generic map(
      LOC => "SLICE_X8Y43",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_84_rt_3259,
      O => SPI_mod_lookup_buffer(100),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_115 : X_FF
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_115_dpot_3278,
      O => SPI_MOD_LOOKUP_DATA(115),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_115_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(99),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(115),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_115_dpot_3278
    );
  SPI_MOD_LOOKUP_DATA_114 : X_FF
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_114_dpot_3287,
      O => SPI_MOD_LOOKUP_DATA(114),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_114_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(98),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(114),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_114_dpot_3287
    );
  SPI_MOD_LOOKUP_DATA_113 : X_FF
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_113_dpot_3294,
      O => SPI_MOD_LOOKUP_DATA(113),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_113_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(97),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(113),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_113_dpot_3294
    );
  SPI_MOD_LOOKUP_DATA_112 : X_FF
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_113,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_112_dpot_3271,
      O => SPI_MOD_LOOKUP_DATA(112),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_112_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X8Y44",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(96),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(112),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_112_dpot_3271
    );
  SPI_mod_lookup_buffer_115 : X_FF
    generic map(
      LOC => "SLICE_X8Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(99),
      O => SPI_mod_lookup_buffer(115),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_114 : X_FF
    generic map(
      LOC => "SLICE_X8Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(98),
      O => SPI_mod_lookup_buffer(114),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_113 : X_FF
    generic map(
      LOC => "SLICE_X8Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(97),
      O => SPI_mod_lookup_buffer(113),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_112 : X_FF
    generic map(
      LOC => "SLICE_X8Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(96),
      O => SPI_mod_lookup_buffer(112),
      RST => GND,
      SET => GND
    );
  LO1_LE_OBUF_LO1_LE_OBUF_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => LO1_CLK_OBUF_3308,
      O => LO1_CLK_OBUF_0
    );
  Mmux_LO1_LE11 : X_LUT6
    generic map(
      LOC => "SLICE_X9Y2",
      INIT => X"FF000000FF000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Sync_AUX2_sync_line(2),
      ADR4 => MCU_NSS_IBUF_0,
      ADR5 => '1',
      O => LO1_LE_OBUF_5957
    );
  Mmux_LO1_CLK11 : X_LUT5
    generic map(
      LOC => "SLICE_X9Y2",
      INIT => X"F000F000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => MCU_SCK_IBUF_0,
      ADR3 => Sync_AUX2_sync_line(2),
      ADR4 => '1',
      O => LO1_CLK_OBUF_3308
    );
  SPI_MOD_PHASE_INC_3_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y15",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(3),
      ADR1 => SPI_SPI_BUF_OUT(3),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_3_dpot_6536
    );
  SPI_MOD_PHASE_INC_2_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y15",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(2),
      ADR1 => SPI_SPI_BUF_OUT(2),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_2_dpot_6535
    );
  SPI_MOD_PHASE_INC_2 : X_FF
    generic map(
      LOC => "SLICE_X9Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_2_dpot1_3328,
      O => SPI_MOD_PHASE_INC(2),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_2_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X9Y16",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(2),
      ADR4 => SPI_MOD_PHASE_INC_2_dpot_6535,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_2_dpot1_3328
    );
  SPI_MOD_PHASE_INC_1 : X_FF
    generic map(
      LOC => "SLICE_X9Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_1_dpot1_3335,
      O => SPI_MOD_PHASE_INC(1),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_1_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X9Y16",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(1),
      ADR4 => SPI_MOD_PHASE_INC_1_dpot_6534,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_1_dpot1_3335
    );
  SPI_n0539_inv11_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y16",
      INIT => X"0300000000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_selected_register(4),
      ADR2 => SPI_selected_register(3),
      ADR3 => SPI_SPI_COMPLETE_6402,
      ADR4 => SPI_state_FSM_FFd2_6400,
      ADR5 => SPI_state_FSM_FFd1_5903,
      O => SPI_n0539_inv11_rstpot_6453
    );
  SPI_MOD_PHASE_INC_0 : X_FF
    generic map(
      LOC => "SLICE_X9Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_0_dpot1_3346,
      O => SPI_MOD_PHASE_INC(0),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_0_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X9Y16",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(0),
      ADR4 => SPI_MOD_PHASE_INC_0_dpot_6652,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_0_dpot1_3346
    );
  SPI_mod_lookup_buffer_3_SPI_mod_lookup_buffer_3_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(19),
      O => SPI_mod_lookup_buffer_19_0
    );
  SPI_mod_lookup_buffer_3_SPI_mod_lookup_buffer_3_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(18),
      O => SPI_mod_lookup_buffer_18_0
    );
  SPI_mod_lookup_buffer_3_SPI_mod_lookup_buffer_3_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(17),
      O => SPI_mod_lookup_buffer_17_0
    );
  SPI_mod_lookup_buffer_3_SPI_mod_lookup_buffer_3_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(16),
      O => SPI_mod_lookup_buffer_16_0
    );
  SPI_mod_lookup_buffer_3 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(3),
      O => SPI_mod_lookup_buffer(3),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_3_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(3),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_3_rt_3360
    );
  SPI_mod_lookup_buffer_19 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_3_rt_3360,
      O => SPI_mod_lookup_buffer(19),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_2 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(2),
      O => SPI_mod_lookup_buffer(2),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_2_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(2),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_2_rt_3367
    );
  SPI_mod_lookup_buffer_18 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_2_rt_3367,
      O => SPI_mod_lookup_buffer(18),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_1 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(1),
      O => SPI_mod_lookup_buffer(1),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_1_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(1),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_1_rt_3350
    );
  SPI_mod_lookup_buffer_17 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_1_rt_3350,
      O => SPI_mod_lookup_buffer(17),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_0 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(0),
      O => SPI_mod_lookup_buffer(0),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_0_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(0),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_0_rt_3355
    );
  SPI_mod_lookup_buffer_16 : X_FF
    generic map(
      LOC => "SLICE_X9Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_0_rt_3355,
      O => SPI_mod_lookup_buffer(16),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_23 : X_FF
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_23_dpot_3383,
      O => SPI_MOD_LOOKUP_DATA(23),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_23_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(7),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(23),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_23_dpot_3383
    );
  SPI_MOD_LOOKUP_DATA_22 : X_FF
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_22_dpot_3392,
      O => SPI_MOD_LOOKUP_DATA(22),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_22_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(6),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(22),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_22_dpot_3392
    );
  SPI_MOD_LOOKUP_DATA_21 : X_FF
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_21_dpot_3399,
      O => SPI_MOD_LOOKUP_DATA(21),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_21_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(5),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(21),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_21_dpot_3399
    );
  SPI_MOD_LOOKUP_DATA_20 : X_FF
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_1,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_20_dpot_3376,
      O => SPI_MOD_LOOKUP_DATA(20),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_20_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y32",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(4),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(20),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_20_dpot_3376
    );
  SPI_n0453_inv1 : X_LUT6
    generic map(
      LOC => "SLICE_X9Y34",
      INIT => X"0000000100000000"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(13),
      ADR1 => SPI_SPI_BUF_OUT(15),
      ADR2 => SPI_SPI_BUF_OUT(14),
      ADR3 => SPI_state_FSM_FFd2_6400,
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_n0419_13_11_6691,
      O => SPI_n0453_inv
    );
  SPI_SOURCE_ATTENUATION_6 : X_FF
    generic map(
      LOC => "SLICE_X9Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_ATTENUATION_6_dpot_3406,
      O => SPI_SOURCE_ATTENUATION(6),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_6_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y34",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_ATTENUATION(6),
      ADR4 => SPI_SPI_BUF_OUT(11),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_ATTENUATION_6_dpot_3406
    );
  SPI_SOURCE_ATTENUATION_5 : X_FF
    generic map(
      LOC => "SLICE_X9Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_ATTENUATION_5_dpot_3411,
      O => SPI_SOURCE_ATTENUATION(5),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_5_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y34",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_ATTENUATION(5),
      ADR4 => SPI_SPI_BUF_OUT(10),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_ATTENUATION_5_dpot_3411
    );
  SPI_SOURCE_ATTENUATION_4 : X_FF
    generic map(
      LOC => "SLICE_X9Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_ATTENUATION_4_dpot_3416,
      O => SPI_SOURCE_ATTENUATION(4),
      RST => GND,
      SET => GND
    );
  SPI_SOURCE_ATTENUATION_4_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y34",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_ATTENUATION(4),
      ADR4 => SPI_SPI_BUF_OUT(9),
      ADR5 => SPI_n0487_inv_rstpot_6436,
      O => SPI_SOURCE_ATTENUATION_4_dpot_3416
    );
  SPI_n0419_13_11_3 : X_LUT6
    generic map(
      LOC => "SLICE_X9Y35",
      INIT => X"00000000FFFD0000"
    )
    port map (
      ADR0 => SPI_last_NSS_5865,
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => Sync_AUX1_sync_line(2),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_n0419_13_112
    );
  SPI_MOD_LOOKUP_DATA_47 : X_FF
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_47_dpot_3444,
      O => SPI_MOD_LOOKUP_DATA(47),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_47_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_31_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(47),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_47_dpot_3444
    );
  SPI_MOD_LOOKUP_DATA_46 : X_FF
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_46_dpot_3453,
      O => SPI_MOD_LOOKUP_DATA(46),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_46_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_30_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(46),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_46_dpot_3453
    );
  SPI_MOD_LOOKUP_DATA_45 : X_FF
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_45_dpot_3460,
      O => SPI_MOD_LOOKUP_DATA(45),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_45_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_29_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(45),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_45_dpot_3460
    );
  SPI_MOD_LOOKUP_DATA_44 : X_FF
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_44_dpot_3437,
      O => SPI_MOD_LOOKUP_DATA(44),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_44_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X9Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_28_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(44),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_44_dpot_3437
    );
  SPI_mod_lookup_buffer_47_SPI_mod_lookup_buffer_47_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(63),
      O => SPI_mod_lookup_buffer_63_0
    );
  SPI_mod_lookup_buffer_47_SPI_mod_lookup_buffer_47_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(62),
      O => SPI_mod_lookup_buffer_62_0
    );
  SPI_mod_lookup_buffer_47_SPI_mod_lookup_buffer_47_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(61),
      O => SPI_mod_lookup_buffer_61_0
    );
  SPI_mod_lookup_buffer_47_SPI_mod_lookup_buffer_47_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(60),
      O => SPI_mod_lookup_buffer_60_0
    );
  SPI_mod_lookup_buffer_47 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_31_0,
      O => SPI_mod_lookup_buffer(47),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_47_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(47),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_47_rt_3473
    );
  SPI_mod_lookup_buffer_63 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_47_rt_3473,
      O => SPI_mod_lookup_buffer(63),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_46 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_30_0,
      O => SPI_mod_lookup_buffer(46),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_46_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(46),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_46_rt_3480
    );
  SPI_mod_lookup_buffer_62 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_46_rt_3480,
      O => SPI_mod_lookup_buffer(62),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_45 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_29_0,
      O => SPI_mod_lookup_buffer(45),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_45_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(45),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_45_rt_3463
    );
  SPI_mod_lookup_buffer_61 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_45_rt_3463,
      O => SPI_mod_lookup_buffer(61),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_44 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_28_0,
      O => SPI_mod_lookup_buffer(44),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_44_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(44),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_44_rt_3468
    );
  SPI_mod_lookup_buffer_60 : X_FF
    generic map(
      LOC => "SLICE_X9Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_44_rt_3468,
      O => SPI_mod_lookup_buffer(60),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_59 : X_FF
    generic map(
      LOC => "SLICE_X9Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_43_0,
      O => SPI_mod_lookup_buffer(59),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_58 : X_FF
    generic map(
      LOC => "SLICE_X9Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_42_0,
      O => SPI_mod_lookup_buffer(58),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_57 : X_FF
    generic map(
      LOC => "SLICE_X9Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_41_0,
      O => SPI_mod_lookup_buffer(57),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_56 : X_FF
    generic map(
      LOC => "SLICE_X9Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_40_0,
      O => SPI_mod_lookup_buffer(56),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_83 : X_FF
    generic map(
      LOC => "SLICE_X9Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_67_0,
      O => SPI_mod_lookup_buffer(83),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_82 : X_FF
    generic map(
      LOC => "SLICE_X9Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_66_0,
      O => SPI_mod_lookup_buffer(82),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_81 : X_FF
    generic map(
      LOC => "SLICE_X9Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_65_0,
      O => SPI_mod_lookup_buffer(81),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_80 : X_FF
    generic map(
      LOC => "SLICE_X9Y40",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv11,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_64_0,
      O => SPI_mod_lookup_buffer(80),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_131_SPI_MOD_LOOKUP_DATA_131_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_DATA(135),
      O => SPI_MOD_LOOKUP_DATA_135_0
    );
  SPI_MOD_LOOKUP_DATA_131_SPI_MOD_LOOKUP_DATA_131_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_DATA(134),
      O => SPI_MOD_LOOKUP_DATA_134_0
    );
  SPI_MOD_LOOKUP_DATA_131_SPI_MOD_LOOKUP_DATA_131_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_DATA(133),
      O => SPI_MOD_LOOKUP_DATA_133_0
    );
  SPI_MOD_LOOKUP_DATA_131_SPI_MOD_LOOKUP_DATA_131_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_LOOKUP_DATA(132),
      O => SPI_MOD_LOOKUP_DATA_132_0
    );
  SPI_MOD_LOOKUP_DATA_131 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(115),
      O => SPI_MOD_LOOKUP_DATA(131),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_119_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(119),
      O => SPI_mod_lookup_buffer_119_rt_3522
    );
  SPI_MOD_LOOKUP_DATA_135 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_119_rt_3522,
      O => SPI_MOD_LOOKUP_DATA(135),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_130 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(114),
      O => SPI_MOD_LOOKUP_DATA(130),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_118_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(118),
      O => SPI_mod_lookup_buffer_118_rt_3529
    );
  SPI_MOD_LOOKUP_DATA_134 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_118_rt_3529,
      O => SPI_MOD_LOOKUP_DATA(134),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_129 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(113),
      O => SPI_MOD_LOOKUP_DATA(129),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_117_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer(117),
      O => SPI_mod_lookup_buffer_117_rt_3507
    );
  SPI_MOD_LOOKUP_DATA_133 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_117_rt_3507,
      O => SPI_MOD_LOOKUP_DATA(133),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_128 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(112),
      O => SPI_MOD_LOOKUP_DATA(128),
      RST => GND,
      SET => GND
    );
  SPI_n0565_inv1_4 : X_LUT6
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => X"00C0000000C00000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_6694,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_state_FSM_FFd1_5903,
      ADR4 => SPI_n0419_13_11_6691,
      ADR5 => '1',
      O => SPI_n0565_inv13
    );
  SPI_mod_lookup_buffer_116_rt : X_LUT5
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => X"AAAAAAAA"
    )
    port map (
      ADR0 => SPI_mod_lookup_buffer(116),
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_116_rt_3513
    );
  SPI_MOD_LOOKUP_DATA_132 : X_FF
    generic map(
      LOC => "SLICE_X9Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0565_inv13,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_116_rt_3513,
      O => SPI_MOD_LOOKUP_DATA(132),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_3 : X_FF
    generic map(
      LOC => "SLICE_X10Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(2),
      O => SPI_SPI_data(3),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_2 : X_FF
    generic map(
      LOC => "SLICE_X10Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(1),
      O => SPI_SPI_data(2),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_1 : X_FF
    generic map(
      LOC => "SLICE_X10Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(0),
      O => SPI_SPI_data(1),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_0 : X_FF
    generic map(
      LOC => "SLICE_X10Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => MCU_MOSI_IBUF_0,
      O => SPI_SPI_data(0),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_7 : X_FF
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(7),
      O => SPI_SPI_BUF_OUT(7),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_5_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(5),
      ADR1 => SPI_SPI_BUF_OUT(5),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_5_dpot_6538
    );
  SPI_SPI_BUF_OUT_6 : X_FF
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(6),
      O => SPI_SPI_BUF_OUT(6),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_4_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(4),
      ADR1 => SPI_SPI_BUF_OUT(4),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_4_dpot_6537
    );
  SPI_SPI_BUF_OUT_5 : X_FF
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(5),
      O => SPI_SPI_BUF_OUT(5),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_6_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(6),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(6),
      O => SPI_MOD_FIFO_THRESHOLD_6_dpot_5924
    );
  SPI_SPI_BUF_OUT_4 : X_FF
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(4),
      O => SPI_SPI_BUF_OUT(4),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_4_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y18",
      INIT => X"AAAAAAAAABA8AAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(4),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_SPI_BUF_OUT(4),
      ADR4 => SPI_selected_register(2),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_FIFO_THRESHOLD_4_dpot_5825
    );
  SPI_SPI_mosi_buffer_14 : X_FF
    generic map(
      LOC => "SLICE_X10Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(13),
      O => SPI_SPI_mosi_buffer(14),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_13 : X_FF
    generic map(
      LOC => "SLICE_X10Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(12),
      O => SPI_SPI_mosi_buffer(13),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_12 : X_FF
    generic map(
      LOC => "SLICE_X10Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer_11_0,
      O => SPI_SPI_mosi_buffer(12),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_15 : X_FF
    generic map(
      LOC => "SLICE_X10Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(15),
      O => SPI_SPI_BUF_OUT(15),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_14 : X_FF
    generic map(
      LOC => "SLICE_X10Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(14),
      O => SPI_SPI_BUF_OUT(14),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_13 : X_FF
    generic map(
      LOC => "SLICE_X10Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(13),
      O => SPI_SPI_BUF_OUT(13),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_12 : X_FF
    generic map(
      LOC => "SLICE_X10Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(12),
      O => SPI_SPI_BUF_OUT(12),
      RST => GND,
      SET => GND
    );
  SPI_SPI_BUF_OUT_11 : X_FF
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data_11_0,
      O => SPI_SPI_BUF_OUT(11),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_10_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(10),
      ADR1 => SPI_SPI_BUF_OUT(10),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_10_dpot_6452
    );
  SPI_SPI_BUF_OUT_10 : X_FF
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data_10_0,
      O => SPI_SPI_BUF_OUT(10),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_9_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(9),
      ADR1 => SPI_SPI_BUF_OUT(9),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_9_dpot_6542
    );
  SPI_SPI_BUF_OUT_9 : X_FF
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data_9_0,
      O => SPI_SPI_BUF_OUT(9),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_8_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(8),
      ADR1 => SPI_SPI_BUF_OUT(8),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_8_dpot_6541
    );
  SPI_SPI_BUF_OUT_8 : X_FF
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data_8_0,
      O => SPI_SPI_BUF_OUT(8),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_8_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y21",
      INIT => X"AAAAAAAAABA8AAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(8),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_SPI_BUF_OUT(8),
      ADR4 => SPI_selected_register(2),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_FIFO_THRESHOLD_8_dpot_5834
    );
  SPI_n0539_inv11_cepot_SPI_n0539_inv11_cepot_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_n0487_inv_cepot_3638,
      O => SPI_n0487_inv_cepot_0
    );
  SPI_n0539_inv11_cepot : X_LUT6
    generic map(
      LOC => "SLICE_X10Y25",
      INIT => X"FFFFFCFFFFFFFCFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => SPI_last_NSS_5865,
      ADR4 => Sync_AUX1_sync_line(2),
      ADR5 => '1',
      O => SPI_n0539_inv11_cepot_5955
    );
  SPI_n0487_inv_cepot : X_LUT5
    generic map(
      LOC => "SLICE_X10Y25",
      INIT => X"FFFFFCFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => SPI_last_NSS_5865,
      ADR4 => Sync_AUX1_sync_line(2),
      O => SPI_n0487_inv_cepot_3638
    );
  SPI_mod_lookup_buffer_119 : X_FF
    generic map(
      LOC => "SLICE_X10Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_103_0,
      O => SPI_mod_lookup_buffer(119),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_118 : X_FF
    generic map(
      LOC => "SLICE_X10Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_102_0,
      O => SPI_mod_lookup_buffer(118),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_117 : X_FF
    generic map(
      LOC => "SLICE_X10Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_101_0,
      O => SPI_mod_lookup_buffer(117),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_116 : X_FF
    generic map(
      LOC => "SLICE_X10Y45",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv12,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_100_0,
      O => SPI_mod_lookup_buffer(116),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_6_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X11Y15",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(6),
      ADR1 => SPI_SPI_BUF_OUT(6),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_6_dpot_6539
    );
  SPI_SPI_BUF_OUT_3 : X_FF
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(3),
      O => SPI_SPI_BUF_OUT(3),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_1_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => X"AAAAAAAAAACAAAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(1),
      ADR1 => SPI_SPI_BUF_OUT(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_1_dpot_6534
    );
  SPI_SPI_BUF_OUT_2 : X_FF
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(2),
      O => SPI_SPI_BUF_OUT(2),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_0_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(0),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(0),
      O => SPI_MOD_FIFO_THRESHOLD_0_dpot_6690
    );
  SPI_SPI_BUF_OUT_1 : X_FF
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(1),
      O => SPI_SPI_BUF_OUT(1),
      RST => GND,
      SET => GND
    );
  SPI_Mmux_n062111 : X_LUT6
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => X"FFFFFF0000FF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_selected_register(1),
      ADR4 => SPI_SPI_BUF_OUT(12),
      ADR5 => SPI_SPI_BUF_OUT(0),
      O => SPI_n0621
    );
  SPI_SPI_BUF_OUT_0 : X_FF
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0059_inv,
      CLK => clk_pll,
      I => SPI_SPI_data(0),
      O => SPI_SPI_BUF_OUT(0),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_0_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X11Y16",
      INIT => X"AAAAAAAAAEA2AAAA"
    )
    port map (
      ADR0 => SPI_MOD_PHASE_INC(0),
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(2),
      ADR3 => SPI_SPI_BUF_OUT(0),
      ADR4 => SPI_selected_register(1),
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_MOD_PHASE_INC_0_dpot_6652
    );
  SPI_SPI_data_7 : X_FF
    generic map(
      LOC => "SLICE_X11Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(6),
      O => SPI_SPI_data(7),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_6 : X_FF
    generic map(
      LOC => "SLICE_X11Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(5),
      O => SPI_SPI_data(6),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_5 : X_FF
    generic map(
      LOC => "SLICE_X11Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(4),
      O => SPI_SPI_data(5),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_4 : X_FF
    generic map(
      LOC => "SLICE_X11Y18",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(3),
      O => SPI_SPI_data(4),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_7_SPI_SPI_mosi_buffer_7_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_mosi_buffer(11),
      O => SPI_SPI_mosi_buffer_11_0
    );
  SPI_SPI_mosi_buffer_7_SPI_SPI_mosi_buffer_7_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_mosi_buffer_10_pack_5,
      O => SPI_SPI_mosi_buffer(10)
    );
  SPI_SPI_mosi_buffer_7_SPI_SPI_mosi_buffer_7_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_mosi_buffer_9_pack_3,
      O => SPI_SPI_mosi_buffer(9)
    );
  SPI_SPI_mosi_buffer_7_SPI_SPI_mosi_buffer_7_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_mosi_buffer_8_pack_1,
      O => SPI_SPI_mosi_buffer(8)
    );
  SPI_SPI_mosi_buffer_7 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(6),
      O => SPI_SPI_mosi_buffer(7),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_10_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(10),
      O => SPI_SPI_mosi_buffer_10_rt_3708
    );
  SPI_SPI_mosi_buffer_11 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer_10_rt_3708,
      O => SPI_SPI_mosi_buffer(11),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_6 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(5),
      O => SPI_SPI_mosi_buffer(6),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_9_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(9),
      O => SPI_SPI_mosi_buffer_9_rt_3715
    );
  SPI_SPI_mosi_buffer_10 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer_9_rt_3715,
      O => SPI_SPI_mosi_buffer_10_pack_5,
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_5 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(4),
      O => SPI_SPI_mosi_buffer(5),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_8_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(8),
      O => SPI_SPI_mosi_buffer_8_rt_3720
    );
  SPI_SPI_mosi_buffer_9 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer_8_rt_3720,
      O => SPI_SPI_mosi_buffer_9_pack_3,
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_4 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(3),
      O => SPI_SPI_mosi_buffer(4),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_7_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(7),
      O => SPI_SPI_mosi_buffer_7_rt_3725
    );
  SPI_SPI_mosi_buffer_8 : X_FF
    generic map(
      LOC => "SLICE_X11Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer_7_rt_3725,
      O => SPI_SPI_mosi_buffer_8_pack_1,
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_15_SPI_SPI_data_15_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_data(11),
      O => SPI_SPI_data_11_0
    );
  SPI_SPI_data_15_SPI_SPI_data_15_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_data(10),
      O => SPI_SPI_data_10_0
    );
  SPI_SPI_data_15_SPI_SPI_data_15_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_data(9),
      O => SPI_SPI_data_9_0
    );
  SPI_SPI_data_15_SPI_SPI_data_15_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_data(8),
      O => SPI_SPI_data_8_0
    );
  SPI_SPI_data_15 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(14),
      O => SPI_SPI_data(15),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_15_SPI_SPI_mosi_buffer_10_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(10),
      O => SPI_SPI_data_15_SPI_SPI_mosi_buffer_10_rt_3730
    );
  SPI_SPI_data_11 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_data_15_SPI_SPI_mosi_buffer_10_rt_3730,
      O => SPI_SPI_data(11),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_14 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(13),
      O => SPI_SPI_data(14),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_15_SPI_SPI_mosi_buffer_9_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(9),
      O => SPI_SPI_data_15_SPI_SPI_mosi_buffer_9_rt_3737
    );
  SPI_SPI_data_10 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_data_15_SPI_SPI_mosi_buffer_9_rt_3737,
      O => SPI_SPI_data(10),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_13 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(12),
      O => SPI_SPI_data(13),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_15_SPI_SPI_mosi_buffer_8_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(8),
      O => SPI_SPI_data_15_SPI_SPI_mosi_buffer_8_rt_3742
    );
  SPI_SPI_data_9 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_data_15_SPI_SPI_mosi_buffer_8_rt_3742,
      O => SPI_SPI_data(9),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_12 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer_11_0,
      O => SPI_SPI_data(12),
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_15_SPI_SPI_mosi_buffer_7_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_mosi_buffer(7),
      O => SPI_SPI_data_15_SPI_SPI_mosi_buffer_7_rt_3747
    );
  SPI_SPI_data_8 : X_FF
    generic map(
      LOC => "SLICE_X11Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_data_15_SPI_SPI_mosi_buffer_7_rt_3747,
      O => SPI_SPI_data(8),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_10 : X_FF
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_10_dpot1_3757,
      O => SPI_MOD_PHASE_INC(10),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_10_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(10),
      ADR4 => SPI_MOD_PHASE_INC_10_dpot_6452,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_10_dpot1_3757
    );
  SPI_MOD_PHASE_INC_9 : X_FF
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_9_dpot1_3764,
      O => SPI_MOD_PHASE_INC(9),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_9_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(9),
      ADR4 => SPI_MOD_PHASE_INC_9_dpot_6542,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_9_dpot1_3764
    );
  SPI_MOD_PHASE_INC_8 : X_FF
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_8_dpot1_3769,
      O => SPI_MOD_PHASE_INC(8),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_8_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(8),
      ADR4 => SPI_MOD_PHASE_INC_8_dpot_6541,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_8_dpot1_3769
    );
  SPI_MOD_PHASE_INC_7 : X_FF
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_cepot_5955,
      CLK => clk_pll,
      I => SPI_MOD_PHASE_INC_7_dpot1_3752,
      O => SPI_MOD_PHASE_INC(7),
      RST => GND,
      SET => GND
    );
  SPI_MOD_PHASE_INC_7_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X11Y21",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_PHASE_INC(7),
      ADR4 => SPI_MOD_PHASE_INC_7_dpot_6540,
      ADR5 => SPI_n0539_inv11_rstpot_6453,
      O => SPI_MOD_PHASE_INC_7_dpot1_3752
    );
  SPI_mod_lookup_buffer_7_SPI_mod_lookup_buffer_7_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(23),
      O => SPI_mod_lookup_buffer_23_0
    );
  SPI_mod_lookup_buffer_7_SPI_mod_lookup_buffer_7_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(22),
      O => SPI_mod_lookup_buffer_22_0
    );
  SPI_mod_lookup_buffer_7_SPI_mod_lookup_buffer_7_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(21),
      O => SPI_mod_lookup_buffer_21_0
    );
  SPI_mod_lookup_buffer_7_SPI_mod_lookup_buffer_7_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(20),
      O => SPI_mod_lookup_buffer_20_0
    );
  SPI_mod_lookup_buffer_7 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(7),
      O => SPI_mod_lookup_buffer(7),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_7_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(7),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_7_rt_3782
    );
  SPI_mod_lookup_buffer_23 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_7_rt_3782,
      O => SPI_mod_lookup_buffer(23),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_6 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(6),
      O => SPI_mod_lookup_buffer(6),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_6_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(6),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_6_rt_3789
    );
  SPI_mod_lookup_buffer_22 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_6_rt_3789,
      O => SPI_mod_lookup_buffer(22),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_5 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(5),
      O => SPI_mod_lookup_buffer(5),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_5_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(5),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_5_rt_3772
    );
  SPI_mod_lookup_buffer_21 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_5_rt_3772,
      O => SPI_mod_lookup_buffer(21),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_4 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(4),
      O => SPI_mod_lookup_buffer(4),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_4_rt : X_LUT5
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(4),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_4_rt_3777
    );
  SPI_mod_lookup_buffer_20 : X_FF
    generic map(
      LOC => "SLICE_X11Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_4_rt_3777,
      O => SPI_mod_lookup_buffer(20),
      RST => GND,
      SET => GND
    );
  INV_SPI_SPI_miso_buffer_8CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_8CLK
    );
  INV_SPI_SPI_miso_buffer_14CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_14CLK
    );
  INV_SPI_SPI_miso_buffer_13CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_13CLK
    );
  SPI_SPI_miso_buffer_14_SPI_SPI_miso_buffer_14_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_miso_buffer(8),
      O => SPI_SPI_miso_buffer_8_0
    );
  SPI_SPI_Mcount_bit_cnt31 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y7",
      INIT => X"3CCCCCCC3CCCCCCC"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(0),
      ADR4 => SPI_SPI_bit_cnt(1),
      ADR5 => '1',
      O => SPI_SPI_Mcount_bit_cnt3
    );
  SPI_SPI_mux1411 : X_LUT5
    generic map(
      LOC => "SLICE_X12Y7",
      INIT => X"AAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(7),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(0),
      ADR4 => SPI_SPI_bit_cnt(1),
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_8_Q
    );
  SPI_SPI_miso_buffer_8 : X_FF
    generic map(
      LOC => "SLICE_X12Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_8CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_8_Q,
      O => SPI_SPI_miso_buffer(8),
      RST => GND,
      SET => GND
    );
  SPI_SPI_miso_buffer_14 : X_FF
    generic map(
      LOC => "SLICE_X12Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_14CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_14_Q,
      O => SPI_SPI_miso_buffer(14),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux511 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y7",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(13),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_15_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_14_Q
    );
  SPI_SPI_miso_buffer_13 : X_FF
    generic map(
      LOC => "SLICE_X12Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_14_INV_SPI_SPI_miso_buffer_13CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_13_Q,
      O => SPI_SPI_miso_buffer(13),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux411 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y7",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(12),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_15_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_13_Q
    );
  INV_SPI_SPI_miso_buffer_11CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => N87_INV_SPI_SPI_miso_buffer_11CLK
    );
  INV_SPI_SPI_miso_buffer_10CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => N87_INV_SPI_SPI_miso_buffer_10CLK
    );
  INV_SPI_SPI_miso_buffer_9CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => N87_INV_SPI_SPI_miso_buffer_9CLK
    );
  N87_N87_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_miso_buffer(11),
      O => SPI_SPI_miso_buffer_11_0
    );
  N87_N87_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_miso_buffer_10_pack_10,
      O => SPI_SPI_miso_buffer(10)
    );
  N87_N87_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_miso_buffer_9_pack_9,
      O => SPI_SPI_miso_buffer(9)
    );
  SPI_SPI_data_valid_0_rstpot_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => X"FF000000FF000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SPI_bit_cnt(2),
      ADR4 => SPI_SPI_bit_cnt(3),
      ADR5 => '1',
      O => N87
    );
  SPI_SPI_mux211 : X_LUT5
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => X"AAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(10),
      ADR1 => SPI_SPI_bit_cnt(1),
      ADR2 => SPI_SPI_bit_cnt(0),
      ADR3 => SPI_SPI_bit_cnt(2),
      ADR4 => SPI_SPI_bit_cnt(3),
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_11_Q
    );
  SPI_SPI_miso_buffer_11 : X_FF
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => N87_INV_SPI_SPI_miso_buffer_11CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_11_Q,
      O => SPI_SPI_miso_buffer(11),
      RST => GND,
      SET => GND
    );
  SPI_SPI_Mcount_bit_cnt11 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => X"00FFFF0000FFFF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SPI_bit_cnt(0),
      ADR4 => SPI_SPI_bit_cnt(1),
      ADR5 => '1',
      O => SPI_SPI_Mcount_bit_cnt1
    );
  SPI_SPI_mux1112 : X_LUT5
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => X"AAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(9),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(0),
      ADR4 => SPI_SPI_bit_cnt(1),
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_10_Q
    );
  SPI_SPI_miso_buffer_10 : X_FF
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => N87_INV_SPI_SPI_miso_buffer_10CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_10_Q,
      O => SPI_SPI_miso_buffer_10_pack_10,
      RST => GND,
      SET => GND
    );
  SPI_SPI_GND_18_o_bit_cnt_3_equal_4_o_3_1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => X"0000000300000003"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => '1',
      O => SPI_SPI_GND_18_o_bit_cnt_3_equal_4_o
    );
  SPI_SPI_mux1511 : X_LUT5
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => X"AAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer_8_0,
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_9_Q
    );
  SPI_SPI_miso_buffer_9 : X_FF
    generic map(
      LOC => "SLICE_X12Y8",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => N87_INV_SPI_SPI_miso_buffer_9CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_9_Q,
      O => SPI_SPI_miso_buffer_9_pack_9,
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_valid_0_SPI_SPI_data_valid_0_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv,
      O => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0
    );
  SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y12",
      INIT => X"C0000000C0000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_bit_cnt(0),
      ADR2 => SPI_SPI_bit_cnt(1),
      ADR3 => SPI_SPI_bit_cnt(2),
      ADR4 => SPI_SPI_bit_cnt(3),
      ADR5 => '1',
      O => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o
    );
  SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv11 : X_LUT5
    generic map(
      LOC => "SLICE_X12Y12",
      INIT => X"3FFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_bit_cnt(0),
      ADR2 => SPI_SPI_bit_cnt(1),
      ADR3 => SPI_SPI_bit_cnt(2),
      ADR4 => SPI_SPI_bit_cnt(3),
      O => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv
    );
  SPI_SPI_data_valid_0 : X_FF
    generic map(
      LOC => "SLICE_X12Y12",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_data_valid_0_rstpot_3858,
      O => SPI_SPI_data_valid_0_5893,
      RST => GND,
      SET => GND
    );
  SPI_SPI_data_valid_0_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y12",
      INIT => X"80FFFFFF8000FF00"
    )
    port map (
      ADR0 => N87,
      ADR1 => SPI_SPI_bit_cnt(1),
      ADR2 => SPI_SPI_bit_cnt(0),
      ADR3 => SPI_SPI_data_valid_0_5893,
      ADR4 => SPI_SPI_data_synced_2_5894,
      ADR5 => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o,
      O => SPI_SPI_data_valid_0_rstpot_3858
    );
  SPI_n0419_13_2 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y13",
      INIT => X"0000001000000000"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(13),
      ADR1 => SPI_SPI_BUF_OUT(15),
      ADR2 => SPI_SPI_BUF_OUT(14),
      ADR3 => SPI_state_FSM_FFd2_6400,
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_n0419_13_11_6691,
      O => SPI_n0419
    );
  SPI_spi_buf_in_15 : X_SFF
    generic map(
      LOC => "SLICE_X12Y13",
      INIT => '1'
    )
    port map (
      CE => SPI_n0699_inv,
      CLK => clk_pll,
      I => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_15_Q,
      O => SPI_spi_buf_in_15_Q,
      SSET => SPI_n0419,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  SPI_Mmux_spi_buf_in_15_interrupt_status_15_mux_50_OUT71 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y13",
      INIT => X"FF00FF00FC00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => SPI_spi_buf_in_15_Q,
      ADR4 => SPI_last_NSS_5865,
      ADR5 => Sync_AUX1_sync_line(2),
      O => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_15_Q
    );
  SPI_SPI_data_synced_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y14",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_Mshreg_data_synced_2_3885,
      O => SPI_SPI_data_synced_2_5894,
      RST => GND,
      SET => GND
    );
  SPI_SPI_Mshreg_data_synced_2 : X_SRLC16E
    generic map(
      LOC => "SLICE_X12Y14",
      INIT => X"0000"
    )
    port map (
      A0 => '0',
      A1 => '0',
      A2 => '0',
      A3 => '0',
      CLK => MCU_SCK_IBUF_BUFG_5814,
      D => SPI_SPI_data_synced_0_6630,
      Q15 => NLW_SPI_SPI_Mshreg_data_synced_2_Q15_UNCONNECTED,
      Q => SPI_SPI_Mshreg_data_synced_2_3885,
      CE => '1'
    );
  SPI_SPI_mosi_buffer_3 : X_FF
    generic map(
      LOC => "SLICE_X12Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(2),
      O => SPI_SPI_mosi_buffer(3),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(1),
      O => SPI_SPI_mosi_buffer(2),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => SPI_SPI_mosi_buffer(0),
      O => SPI_SPI_mosi_buffer(1),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mosi_buffer_0 : X_FF
    generic map(
      LOC => "SLICE_X12Y16",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_PWR_13_o_bit_cnt_3_equal_5_o_inv_0,
      CLK => MCU_SCK_IBUF_BUFG_5814,
      I => MCU_MOSI_IBUF_0,
      O => SPI_SPI_mosi_buffer(0),
      RST => GND,
      SET => GND
    );
  Sync_AUX2_sync_line_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX2_sync_line(1),
      O => Sync_AUX2_sync_line(2),
      RST => GND,
      SET => GND
    );
  SPI_n0699_inv1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => X"00000000FFFF0002"
    )
    port map (
      ADR0 => SPI_last_NSS_5865,
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX1_sync_line(2),
      ADR3 => Sync_AUX2_sync_line(2),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_n0699_inv
    );
  Sync_NSS_sync_line_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_NSS_Mshreg_sync_line_1_3913,
      O => Sync_NSS_sync_line(1),
      RST => GND,
      SET => GND
    );
  Sync_NSS_Mshreg_sync_line_1 : X_SRLC16E
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => X"0000"
    )
    port map (
      A0 => '0',
      A1 => '0',
      A2 => '0',
      A3 => '0',
      CLK => clk_pll,
      D => MCU_NSS_IBUF_0,
      Q15 => NLW_Sync_NSS_Mshreg_sync_line_1_Q15_UNCONNECTED,
      Q => Sync_NSS_Mshreg_sync_line_1_3913,
      CE => '1'
    );
  Sync_AUX1_sync_line_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX1_Mshreg_sync_line_1_3920,
      O => Sync_AUX1_sync_line(1),
      RST => GND,
      SET => GND
    );
  Sync_AUX1_Mshreg_sync_line_1 : X_SRLC16E
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => X"0000"
    )
    port map (
      A0 => '0',
      A1 => '0',
      A2 => '0',
      A3 => '0',
      CLK => clk_pll,
      D => MCU_AUX1_IBUF_0,
      Q15 => NLW_Sync_AUX1_Mshreg_sync_line_1_Q15_UNCONNECTED,
      Q => Sync_AUX1_Mshreg_sync_line_1_3920,
      CE => '1'
    );
  Sync_AUX2_sync_line_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX2_Mshreg_sync_line_1_3927,
      O => Sync_AUX2_sync_line(1),
      RST => GND,
      SET => GND
    );
  Sync_AUX2_Mshreg_sync_line_1 : X_SRLC16E
    generic map(
      LOC => "SLICE_X12Y18",
      INIT => X"0000"
    )
    port map (
      A0 => '0',
      A1 => '0',
      A2 => '0',
      A3 => '0',
      CLK => clk_pll,
      D => MCU_AUX2_IBUF_0,
      Q15 => NLW_Sync_AUX2_Mshreg_sync_line_1_Q15_UNCONNECTED,
      Q => Sync_AUX2_Mshreg_sync_line_1_3927,
      CE => '1'
    );
  SPI_SPI_data_valid_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y19",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_SPI_data_valid_0_5893,
      O => SPI_SPI_data_valid_1_6641,
      RST => GND,
      SET => GND
    );
  Sync_AUX2_sync_line_2_2_Sync_AUX2_sync_line_2_2_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Sync_NSS_sync_line(2),
      O => Sync_NSS_sync_line_2_0
    );
  SPI_n0484_inv : X_LUT6
    generic map(
      LOC => "SLICE_X12Y20",
      INIT => X"0000000000001000"
    )
    port map (
      ADR0 => SPI_selected_register(3),
      ADR1 => SPI_selected_register(2),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_state_FSM_FFd1_5903,
      ADR4 => N8,
      ADR5 => SPI_NSS_last_NSS_AND_33_o,
      O => SPI_n0484_inv_6653
    );
  SPI_NSS_last_NSS_AND_33_o1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y20",
      INIT => X"0000000F00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_NSS_sync_line_2_1_6663,
      ADR3 => Sync_AUX2_sync_line_2_2_6833,
      ADR4 => Sync_AUX1_sync_line_2_2_6639,
      ADR5 => SPI_last_NSS_5865,
      O => SPI_NSS_last_NSS_AND_33_o
    );
  Sync_AUX2_sync_line_2_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y20",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX2_sync_line(1),
      O => Sync_AUX2_sync_line_2_2_6833,
      RST => GND,
      SET => GND
    );
  Sync_AUX2_sync_line_2_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y20",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX2_sync_line(1),
      O => Sync_AUX2_sync_line_2_1_6692,
      RST => GND,
      SET => GND
    );
  SPI_SPI_n0059_inv1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y20",
      INIT => X"0000FF000000FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SPI_data_valid_1_6641,
      ADR4 => SPI_SPI_data_synced_0_6630,
      ADR5 => '1',
      O => SPI_SPI_n0059_inv
    );
  Sync_NSS_sync_line_1_rt : X_LUT5
    generic map(
      LOC => "SLICE_X12Y20",
      INIT => X"F0F0F0F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_NSS_sync_line(1),
      ADR3 => '1',
      ADR4 => '1',
      O => Sync_NSS_sync_line_1_rt_3963
    );
  Sync_NSS_sync_line_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y20",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_NSS_sync_line_1_rt_3963,
      O => Sync_NSS_sync_line(2),
      RST => GND,
      SET => GND
    );
  Sync_AUX1_sync_line_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y21",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX1_sync_line(1),
      O => Sync_AUX1_sync_line(2),
      RST => GND,
      SET => GND
    );
  SPI_n0474_inv_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y21",
      INIT => X"FFFFFFFFFFFF00FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SPI_BUF_OUT(15),
      ADR4 => SPI_SPI_BUF_OUT(13),
      ADR5 => SPI_SPI_BUF_OUT(14),
      O => N12
    );
  Sync_NSS_sync_line_2_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y21",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_NSS_sync_line(1),
      O => Sync_NSS_sync_line_2_1_6663,
      RST => GND,
      SET => GND
    );
  Sync_AUX1_sync_line_2_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y22",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX1_sync_line(1),
      O => Sync_AUX1_sync_line_2_2_6639,
      RST => GND,
      SET => GND
    );
  Sync_AUX1_sync_line_2_1 : X_FF
    generic map(
      LOC => "SLICE_X12Y22",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX1_sync_line(1),
      O => Sync_AUX1_sync_line_2_1_6638,
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_2_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X12Y25"
    )
    port map (
      IA => N101,
      IB => N102,
      O => SPI_MOD_FIFO_DATA_2_dpot_3983,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_2_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X12Y25",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(2),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(10),
      O => N101
    );
  SPI_MOD_FIFO_DATA_2 : X_FF
    generic map(
      LOC => "SLICE_X12Y25",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_2_dpot_3983,
      O => SPI_MOD_FIFO_DATA(2),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_2_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X12Y25",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(2),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(10),
      O => N102
    );
  SPI_n0578_inv1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y25",
      INIT => X"00000F0000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_6694,
      ADR3 => SPI_state_FSM_FFd2_1_6695,
      ADR4 => SPI_state_FSM_FFd1_1_6696,
      ADR5 => SPI_n0419_13_11_6691,
      O => SPI_n0578_inv
    );
  SPI_n0419_13_11_1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y25",
      INIT => X"0F000F000E000F00"
    )
    port map (
      ADR0 => Sync_AUX2_sync_line_2_1_6692,
      ADR1 => Sync_AUX1_sync_line_2_1_6638,
      ADR2 => Inst_ResetDelay_OUT_RESET_1_6659,
      ADR3 => SPI_SPI_COMPLETE_1_6693,
      ADR4 => SPI_last_NSS_5865,
      ADR5 => Sync_NSS_sync_line_2_0,
      O => SPI_n0419_13_11_6691
    );
  SPI_MOD_FIFO_DATA_7_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X12Y26"
    )
    port map (
      IA => N111,
      IB => N112,
      O => SPI_MOD_FIFO_DATA_7_dpot_4014,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_7_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X12Y26",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(7),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(15),
      O => N111
    );
  SPI_MOD_FIFO_DATA_7 : X_FF
    generic map(
      LOC => "SLICE_X12Y26",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_7_dpot_4014,
      O => SPI_MOD_FIFO_DATA(7),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_7_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X12Y26",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(7),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(15),
      O => N112
    );
  SPI_MOD_FIFO_DATA_6_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X12Y27"
    )
    port map (
      IA => N109,
      IB => N110,
      O => SPI_MOD_FIFO_DATA_6_dpot_4033,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_6_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X12Y27",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(6),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(14),
      O => N109
    );
  SPI_MOD_FIFO_DATA_6 : X_FF
    generic map(
      LOC => "SLICE_X12Y27",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_6_dpot_4033,
      O => SPI_MOD_FIFO_DATA(6),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_6_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X12Y27",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(6),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(14),
      O => N110
    );
  SPI_MOD_FIFO_DATA_4_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X12Y28"
    )
    port map (
      IA => N105,
      IB => N106,
      O => SPI_MOD_FIFO_DATA_4_dpot_4052,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_4_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X12Y28",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(4),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(12),
      O => N105
    );
  SPI_MOD_FIFO_DATA_4 : X_FF
    generic map(
      LOC => "SLICE_X12Y28",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_4_dpot_4052,
      O => SPI_MOD_FIFO_DATA(4),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_4_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X12Y28",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(4),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(12),
      O => N106
    );
  SPI_n0394_SPI_n0394_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_n0588_inv1_cepot_4072,
      O => SPI_n0588_inv1_cepot_0
    );
  SPI_n03941 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y29",
      INIT => X"0000000200000002"
    )
    port map (
      ADR0 => SPI_last_NSS_5865,
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => Sync_AUX1_sync_line(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => '1',
      O => SPI_n0394
    );
  SPI_n0588_inv1_cepot : X_LUT5
    generic map(
      LOC => "SLICE_X12Y29",
      INIT => X"FFFDFFFD"
    )
    port map (
      ADR0 => SPI_last_NSS_5865,
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => Sync_AUX1_sync_line(2),
      ADR4 => '1',
      O => SPI_n0588_inv1_cepot_4072
    );
  N14_N14_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SOURCE_FILTER_0_pack_4,
      O => SPI_SOURCE_FILTER(0)
    );
  SPI_state_FSM_FFd1_In_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y31",
      INIT => X"FFFFFF00FFFFFF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SPI_BUF_OUT(14),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => '1',
      O => N14
    );
  SPI_SOURCE_FILTER_0_dpot : X_LUT5
    generic map(
      LOC => "SLICE_X12Y31",
      INIT => X"FC0CFC0C"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SOURCE_FILTER(0),
      ADR2 => SPI_n0487_inv_rstpot_6436,
      ADR3 => SPI_SPI_BUF_OUT(14),
      ADR4 => '1',
      O => SPI_SOURCE_FILTER_0_dpot_4080
    );
  SPI_SOURCE_FILTER_0 : X_FF
    generic map(
      LOC => "SLICE_X12Y31",
      INIT => '0'
    )
    port map (
      CE => SPI_n0487_inv_cepot_0,
      CLK => clk_pll,
      I => SPI_SOURCE_FILTER_0_dpot_4080,
      O => SPI_SOURCE_FILTER_0_pack_4,
      RST => GND,
      SET => GND
    );
  SPI_state_FSM_FFd1_In : X_LUT6
    generic map(
      LOC => "SLICE_X12Y31",
      INIT => X"FFFF0000FFFF0060"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(15),
      ADR1 => SPI_SPI_BUF_OUT(13),
      ADR2 => SPI_SPI_COMPLETE_6402,
      ADR3 => SPI_state_FSM_FFd2_6400,
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => N14,
      O => SPI_state_FSM_FFd1_In_6739
    );
  SPI_n0487_inv_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y31",
      INIT => X"0000000003000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_selected_register(3),
      ADR2 => SPI_selected_register(2),
      ADR3 => SPI_state_FSM_FFd2_6400,
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => N10,
      O => SPI_n0487_inv_rstpot_6436
    );
  SPI_state_FSM_FFd2 : X_SFF
    generic map(
      LOC => "SLICE_X12Y33",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_state_FSM_FFd2_In,
      O => SPI_state_FSM_FFd2_6400,
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_state_FSM_FFd2_In1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y33",
      INIT => X"FF00FF00FF01FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(13),
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => Inst_ResetDelay_OUT_RESET_5868,
      ADR3 => SPI_state_FSM_FFd2_6400,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(14),
      O => SPI_state_FSM_FFd2_In
    );
  SPI_state_FSM_FFd1 : X_SFF
    generic map(
      LOC => "SLICE_X12Y33",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_state_FSM_FFd1_In_6739,
      O => SPI_state_FSM_FFd1_5903,
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_mod_lookup_buffer_35_SPI_mod_lookup_buffer_35_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(43),
      O => SPI_mod_lookup_buffer_43_0
    );
  SPI_mod_lookup_buffer_35_SPI_mod_lookup_buffer_35_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(42),
      O => SPI_mod_lookup_buffer_42_0
    );
  SPI_mod_lookup_buffer_35_SPI_mod_lookup_buffer_35_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(41),
      O => SPI_mod_lookup_buffer_41_0
    );
  SPI_mod_lookup_buffer_35_SPI_mod_lookup_buffer_35_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(40),
      O => SPI_mod_lookup_buffer_40_0
    );
  SPI_mod_lookup_buffer_35 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_19_0,
      O => SPI_mod_lookup_buffer(35),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_27_rt : X_LUT5
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer_27_0,
      O => SPI_mod_lookup_buffer_27_rt_4116
    );
  SPI_mod_lookup_buffer_43 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_27_rt_4116,
      O => SPI_mod_lookup_buffer(43),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_34 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_18_0,
      O => SPI_mod_lookup_buffer(34),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_26_rt : X_LUT5
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer_26_0,
      O => SPI_mod_lookup_buffer_26_rt_4123
    );
  SPI_mod_lookup_buffer_42 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_26_rt_4123,
      O => SPI_mod_lookup_buffer(42),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_33 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_17_0,
      O => SPI_mod_lookup_buffer(33),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_25_rt : X_LUT5
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => X"FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_lookup_buffer_25_0,
      O => SPI_mod_lookup_buffer_25_rt_4128
    );
  SPI_mod_lookup_buffer_41 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_25_rt_4128,
      O => SPI_mod_lookup_buffer(41),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_32 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_16_0,
      O => SPI_mod_lookup_buffer(32),
      RST => GND,
      SET => GND
    );
  SPI_n0578_inv1_1 : X_LUT6
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => X"0030000000300000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_6694,
      ADR2 => SPI_state_FSM_FFd2_1_6695,
      ADR3 => SPI_state_FSM_FFd1_1_6696,
      ADR4 => SPI_n0419_13_11_6691,
      ADR5 => '1',
      O => SPI_n0578_inv1_6656
    );
  SPI_mod_lookup_buffer_24_rt : X_LUT5
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => X"AAAAAAAA"
    )
    port map (
      ADR0 => SPI_mod_lookup_buffer_24_0,
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_24_rt_4134
    );
  SPI_mod_lookup_buffer_40 : X_FF
    generic map(
      LOC => "SLICE_X12Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_24_rt_4134,
      O => SPI_mod_lookup_buffer(40),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_51 : X_FF
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_51_dpot_4153,
      O => SPI_MOD_LOOKUP_DATA(51),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_51_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(35),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(51),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_51_dpot_4153
    );
  SPI_MOD_LOOKUP_DATA_50 : X_FF
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_50_dpot_4162,
      O => SPI_MOD_LOOKUP_DATA(50),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_50_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(34),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(50),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_50_dpot_4162
    );
  SPI_MOD_LOOKUP_DATA_49 : X_FF
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_49_dpot_4169,
      O => SPI_MOD_LOOKUP_DATA(49),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_49_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(33),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(49),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_49_dpot_4169
    );
  SPI_MOD_LOOKUP_DATA_48 : X_FF
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_48_dpot_4146,
      O => SPI_MOD_LOOKUP_DATA(48),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_48_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(32),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(48),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_48_dpot_4146
    );
  SPI_MOD_LOOKUP_DATA_63 : X_FF
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_63_dpot_4183,
      O => SPI_MOD_LOOKUP_DATA(63),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_63_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(47),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(63),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_63_dpot_4183
    );
  SPI_MOD_LOOKUP_DATA_62 : X_FF
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_62_dpot_4192,
      O => SPI_MOD_LOOKUP_DATA(62),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_62_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(46),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(62),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_62_dpot_4192
    );
  SPI_MOD_LOOKUP_DATA_61 : X_FF
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_61_dpot_4199,
      O => SPI_MOD_LOOKUP_DATA(61),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_61_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(45),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(61),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_61_dpot_4199
    );
  SPI_MOD_LOOKUP_DATA_60 : X_FF
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_60_dpot_4176,
      O => SPI_MOD_LOOKUP_DATA(60),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_60_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X12Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(44),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(60),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_60_dpot_4176
    );
  SPI_mod_lookup_buffer_51 : X_FF
    generic map(
      LOC => "SLICE_X12Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(35),
      O => SPI_mod_lookup_buffer(51),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_50 : X_FF
    generic map(
      LOC => "SLICE_X12Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(34),
      O => SPI_mod_lookup_buffer(50),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_49 : X_FF
    generic map(
      LOC => "SLICE_X12Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(33),
      O => SPI_mod_lookup_buffer(49),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_48 : X_FF
    generic map(
      LOC => "SLICE_X12Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer(32),
      O => SPI_mod_lookup_buffer(48),
      RST => GND,
      SET => GND
    );
  INV_SPI_SPI_bit_cnt_3CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_3CLK
    );
  INV_SPI_SPI_bit_cnt_2CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_2CLK
    );
  INV_SPI_SPI_bit_cnt_1CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_1CLK
    );
  INV_SPI_SPI_bit_cnt_0CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_0CLK
    );
  SPI_SPI_bit_cnt_3 : X_FF
    generic map(
      LOC => "SLICE_X13Y7",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_3CLK,
      I => SPI_SPI_Mcount_bit_cnt3,
      O => SPI_SPI_bit_cnt(3),
      RST => fpga_select,
      SET => GND
    );
  SPI_SPI_bit_cnt_2 : X_FF
    generic map(
      LOC => "SLICE_X13Y7",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_2CLK,
      I => SPI_SPI_Mcount_bit_cnt2,
      O => SPI_SPI_bit_cnt(2),
      RST => fpga_select,
      SET => GND
    );
  SPI_SPI_bit_cnt_1 : X_FF
    generic map(
      LOC => "SLICE_X13Y7",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_1CLK,
      I => SPI_SPI_Mcount_bit_cnt1,
      O => SPI_SPI_bit_cnt(1),
      RST => fpga_select,
      SET => GND
    );
  SPI_SPI_bit_cnt_0 : X_FF
    generic map(
      LOC => "SLICE_X13Y7",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => SPI_SPI_bit_cnt_3_INV_SPI_SPI_bit_cnt_0CLK,
      I => SPI_SPI_Mcount_bit_cnt,
      O => SPI_SPI_bit_cnt(0),
      RST => fpga_select,
      SET => GND
    );
  SPI_SPI_Mcount_bit_cnt_xor_0_11_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X13Y7",
      INIT => X"00FF00FF00FF00FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SPI_bit_cnt(0),
      ADR4 => '1',
      ADR5 => '1',
      O => SPI_SPI_Mcount_bit_cnt
    );
  SOURCE_MOSI_OBUF_SOURCE_MOSI_OBUF_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_MISO_OBUFT_4224,
      O => MCU_MISO_OBUFT_0
    );
  Mmux_SOURCE_MOSI11 : X_LUT6
    generic map(
      LOC => "SLICE_X13Y8",
      INIT => X"FFF00F00FFF00F00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX1_sync_line(2),
      ADR3 => Source_latched_regs(127),
      ADR4 => MCU_MOSI_IBUF_0,
      ADR5 => '1',
      O => SOURCE_MOSI_OBUF_6373
    );
  Mmux_Z_3_o_SOURCE_MUX_MUX_194_o12 : X_LUT5
    generic map(
      LOC => "SLICE_X13Y8",
      INIT => X"CACACACA"
    )
    port map (
      ADR0 => Mmux_Z_3_o_SOURCE_MUX_MUX_194_o1,
      ADR1 => SOURCE_MUX_IBUF_0,
      ADR2 => Sync_AUX1_sync_line(2),
      ADR3 => '1',
      ADR4 => '1',
      O => MCU_MISO_OBUFT_4224
    );
  Mmux_Z_3_o_SOURCE_MUX_MUX_194_o11 : X_LUT6
    generic map(
      LOC => "SLICE_X13Y8",
      INIT => X"FFCFFCCC33033000"
    )
    port map (
      ADR0 => '1',
      ADR1 => Sync_AUX2_sync_line(2),
      ADR2 => SPI_SPI_GND_18_o_bit_cnt_3_equal_4_o,
      ADR3 => SPI_spi_buf_in_15_Q,
      ADR4 => SPI_SPI_miso_buffer(14),
      ADR5 => LO1_MUX_IBUF_0,
      O => Mmux_Z_3_o_SOURCE_MUX_MUX_194_o1
    );
  SPI_last_NSS_SPI_last_NSS_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => aux1_sync_aux2_sync_OR_8_o_inv,
      O => aux1_sync_aux2_sync_OR_8_o_inv_0
    );
  SPI_last_NSS : X_SFF
    generic map(
      LOC => "SLICE_X13Y10",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => fpga_select,
      O => SPI_last_NSS_5865,
      SSET => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Mmux_fpga_select11 : X_LUT6
    generic map(
      LOC => "SLICE_X13Y10",
      INIT => X"FFFFFFF0FFFFFFF0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Sync_AUX1_sync_line(2),
      ADR3 => Sync_AUX2_sync_line(2),
      ADR4 => Sync_NSS_sync_line_2_0,
      ADR5 => '1',
      O => fpga_select
    );
  aux1_sync_aux2_sync_OR_8_o_inv1 : X_LUT5
    generic map(
      LOC => "SLICE_X13Y10",
      INIT => X"000C000C"
    )
    port map (
      ADR0 => '1',
      ADR1 => MCU_NSS_IBUF_0,
      ADR2 => Sync_AUX1_sync_line(2),
      ADR3 => Sync_AUX2_sync_line(2),
      ADR4 => '1',
      O => aux1_sync_aux2_sync_OR_8_o_inv
    );
  SPI_SPI_n0066_inv11 : X_LUT6
    generic map(
      LOC => "SLICE_X13Y10",
      INIT => X"0333333333333333"
    )
    port map (
      ADR0 => '1',
      ADR1 => fpga_select,
      ADR2 => SPI_SPI_bit_cnt(0),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(3),
      ADR5 => SPI_SPI_bit_cnt(2),
      O => SPI_SPI_n0066_inv
    );
  SPI_SPI_data_valid_1_inv1_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X13Y20",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => SPI_SPI_data_valid_1_6641,
      O => SPI_SPI_data_valid_1_inv
    );
  SPI_n0474_inv : X_LUT6
    generic map(
      LOC => "SLICE_X13Y21",
      INIT => X"0000000022000002"
    )
    port map (
      ADR0 => SPI_SPI_COMPLETE_6402,
      ADR1 => Inst_ResetDelay_OUT_RESET_5868,
      ADR2 => N12,
      ADR3 => SPI_state_FSM_FFd2_6400,
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_NSS_last_NSS_AND_33_o,
      O => SPI_n0474_inv_5916
    );
  SPI_SPI_data_synced_0 : X_SFF
    generic map(
      LOC => "SLICE_X13Y21",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => '1',
      O => SPI_SPI_data_synced_0_6630,
      SRST => SPI_SPI_data_valid_1_inv,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Inst_ResetDelay_OUT_RESET : X_FF
    generic map(
      LOC => "SLICE_X13Y22",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Inst_ResetDelay_OUT_RESET_rstpot_5930,
      O => Inst_ResetDelay_OUT_RESET_5868,
      RST => GND,
      SET => GND
    );
  Inst_ResetDelay_OUT_RESET_2 : X_FF
    generic map(
      LOC => "SLICE_X13Y23",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Inst_ResetDelay_OUT_RESET_rstpot_5930,
      O => Inst_ResetDelay_OUT_RESET_2_6660,
      RST => GND,
      SET => GND
    );
  Inst_ResetDelay_OUT_RESET_1 : X_FF
    generic map(
      LOC => "SLICE_X13Y23",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Inst_ResetDelay_OUT_RESET_rstpot_5930,
      O => Inst_ResetDelay_OUT_RESET_1_6659,
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_7 : X_FF
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_7_dpot_4284,
      O => SPI_mod_data_LSB(7),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_7_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(7),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(7),
      O => SPI_mod_data_LSB_7_dpot_4284
    );
  SPI_mod_data_LSB_6 : X_FF
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_6_dpot_4294,
      O => SPI_mod_data_LSB(6),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_6_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(6),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(6),
      O => SPI_mod_data_LSB_6_dpot_4294
    );
  SPI_mod_data_LSB_5 : X_FF
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_5_dpot_4302,
      O => SPI_mod_data_LSB(5),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_5_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(5),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(5),
      O => SPI_mod_data_LSB_5_dpot_4302
    );
  SPI_mod_data_LSB_4 : X_FF
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_4_dpot_4277,
      O => SPI_mod_data_LSB(4),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_4_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y26",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(4),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(4),
      O => SPI_mod_data_LSB_4_dpot_4277
    );
  SPI_AMP_SHDN : X_SFF
    generic map(
      LOC => "SLICE_X13Y32",
      INIT => '1'
    )
    port map (
      CE => SPI_n0484_inv_6653,
      CLK => clk_pll,
      I => SPI_SPI_BUF_OUT(0),
      O => SPI_AMP_SHDN_5830,
      SSET => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  SPI_state_FSM_FFd2_1 : X_SFF
    generic map(
      LOC => "SLICE_X13Y33",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_state_FSM_FFd2_In,
      O => SPI_state_FSM_FFd2_1_6695,
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_state_FSM_FFd1_1 : X_SFF
    generic map(
      LOC => "SLICE_X13Y33",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_state_FSM_FFd1_In_6739,
      O => SPI_state_FSM_FFd1_1_6696,
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_n0419_13_11_2 : X_LUT6
    generic map(
      LOC => "SLICE_X13Y34",
      INIT => X"00000000FFFD0000"
    )
    port map (
      ADR0 => SPI_last_NSS_5865,
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => Sync_AUX1_sync_line(2),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_n0419_13_111
    );
  SPI_MOD_LOOKUP_DATA_59 : X_FF
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_59_dpot_4337,
      O => SPI_MOD_LOOKUP_DATA(59),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_59_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_43_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(59),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_59_dpot_4337
    );
  SPI_MOD_LOOKUP_DATA_58 : X_FF
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_58_dpot_4346,
      O => SPI_MOD_LOOKUP_DATA(58),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_58_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_42_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(58),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_58_dpot_4346
    );
  SPI_MOD_LOOKUP_DATA_57 : X_FF
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_57_dpot_4353,
      O => SPI_MOD_LOOKUP_DATA(57),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_57_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_41_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(57),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_57_dpot_4353
    );
  SPI_MOD_LOOKUP_DATA_56 : X_FF
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_56_dpot_4330,
      O => SPI_MOD_LOOKUP_DATA(56),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_56_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X13Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_40_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(56),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_56_dpot_4330
    );
  INV_SPI_SPI_miso_buffer_12CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_12CLK
    );
  INV_SPI_SPI_miso_buffer_7CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_7CLK
    );
  INV_SPI_SPI_miso_buffer_5CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_5CLK
    );
  INV_SPI_SPI_miso_buffer_4CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_4CLK
    );
  SPI_SPI_miso_buffer_12 : X_FF
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_12CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_12_Q,
      O => SPI_SPI_miso_buffer(12),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux311 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer_11_0,
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_15_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_12_Q
    );
  SPI_SPI_miso_buffer_7 : X_FF
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_7CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_7_Q,
      O => SPI_SPI_miso_buffer(7),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux1311 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer_6_0,
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_15_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_7_Q
    );
  SPI_SPI_miso_buffer_5 : X_FF
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_5CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_5_Q,
      O => SPI_SPI_miso_buffer(5),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux11111 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(4),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_15_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_5_Q
    );
  SPI_SPI_miso_buffer_4 : X_FF
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_12_INV_SPI_SPI_miso_buffer_4CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_4_Q,
      O => SPI_SPI_miso_buffer(4),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux1011 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y7",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(3),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_4_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_4_Q
    );
  SPI_spi_buf_in_4 : X_SFF
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => '0'
    )
    port map (
      CE => SPI_n0699_inv,
      CLK => clk_pll,
      I => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_4_Q,
      O => SPI_spi_buf_in_4_Q,
      SRST => SPI_n0419,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_Mmux_spi_buf_in_15_interrupt_status_15_mux_50_OUT111 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => X"FF00FF10FF00EF00"
    )
    port map (
      ADR0 => Sync_NSS_sync_line_2_0,
      ADR1 => Sync_AUX2_sync_line(2),
      ADR2 => SPI_last_NSS_5865,
      ADR3 => SPI_spi_buf_in_4_Q,
      ADR4 => Sync_AUX1_sync_line(2),
      ADR5 => SPI_interrupt_status(4),
      O => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_4_Q
    );
  SPI_spi_buf_in_3 : X_SFF
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => '0'
    )
    port map (
      CE => SPI_n0699_inv,
      CLK => clk_pll,
      I => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_3_Q,
      O => SPI_spi_buf_in_3_Q,
      SRST => SPI_n0419,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_Mmux_spi_buf_in_15_interrupt_status_15_mux_50_OUT101 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => X"FF00FF10FF00EF00"
    )
    port map (
      ADR0 => Sync_NSS_sync_line_2_0,
      ADR1 => Sync_AUX2_sync_line(2),
      ADR2 => SPI_last_NSS_5865,
      ADR3 => SPI_spi_buf_in_3_Q,
      ADR4 => Sync_AUX1_sync_line(2),
      ADR5 => SPI_interrupt_status(3),
      O => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_3_Q
    );
  SPI_spi_buf_in_2 : X_SFF
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => '1'
    )
    port map (
      CE => SPI_n0699_inv,
      CLK => clk_pll,
      I => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_2_Q,
      O => SPI_spi_buf_in_2_Q,
      SSET => SPI_n0419,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  SPI_Mmux_spi_buf_in_15_interrupt_status_15_mux_50_OUT91 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => X"FF00FF10FF00EF00"
    )
    port map (
      ADR0 => Sync_NSS_sync_line_2_0,
      ADR1 => Sync_AUX2_sync_line(2),
      ADR2 => SPI_last_NSS_5865,
      ADR3 => SPI_spi_buf_in_2_Q,
      ADR4 => Sync_AUX1_sync_line(2),
      ADR5 => SPI_interrupt_status(2),
      O => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_2_Q
    );
  SPI_spi_buf_in_1 : X_SFF
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => '0'
    )
    port map (
      CE => SPI_n0699_inv,
      CLK => clk_pll,
      I => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_1_Q,
      O => SPI_spi_buf_in_1_Q,
      SRST => SPI_n0419,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_Mmux_spi_buf_in_15_interrupt_status_15_mux_50_OUT81 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y13",
      INIT => X"FF00FF10FF00EF00"
    )
    port map (
      ADR0 => Sync_NSS_sync_line_2_0,
      ADR1 => Sync_AUX2_sync_line(2),
      ADR2 => SPI_last_NSS_5865,
      ADR3 => SPI_spi_buf_in_1_Q,
      ADR4 => Sync_AUX1_sync_line(2),
      ADR5 => SPI_interrupt_status(1),
      O => SPI_spi_buf_in_15_interrupt_status_15_mux_50_OUT_1_Q
    );
  SPI_interrupt_mask_4 : X_SFF
    generic map(
      LOC => "SLICE_X14Y14",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_1_cepot_0,
      CLK => clk_pll,
      I => SPI_interrupt_mask_4_dpot1_4426,
      O => SPI_interrupt_mask(4),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_interrupt_mask_4_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y14",
      INIT => X"FF00FE02FF00FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(4),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_interrupt_mask(4),
      ADR4 => SPI_selected_register(2),
      ADR5 => SPI_n0539_inv11_1_rstpot_6425,
      O => SPI_interrupt_mask_4_dpot1_4426
    );
  SPI_n0539_inv11_2_cepot_SPI_n0539_inv11_2_cepot_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_n0539_inv11_1_cepot_4437,
      O => SPI_n0539_inv11_1_cepot_0
    );
  SPI_n0539_inv11_2_cepot : X_LUT6
    generic map(
      LOC => "SLICE_X14Y19",
      INIT => X"FFFFFCFFFFFFFCFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => SPI_last_NSS_5865,
      ADR4 => Sync_AUX1_sync_line(2),
      ADR5 => '1',
      O => SPI_n0539_inv11_2_cepot_5823
    );
  SPI_n0539_inv11_1_cepot : X_LUT5
    generic map(
      LOC => "SLICE_X14Y19",
      INIT => X"FFFFFCFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => Sync_NSS_sync_line_2_0,
      ADR2 => Sync_AUX2_sync_line(2),
      ADR3 => SPI_last_NSS_5865,
      ADR4 => Sync_AUX1_sync_line(2),
      O => SPI_n0539_inv11_1_cepot_4437
    );
  SPI_MOD_FIFO_THRESHOLD_8_SPI_MOD_FIFO_THRESHOLD_8_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_FIFO_THRESHOLD_9_pack_8,
      O => SPI_MOD_FIFO_THRESHOLD(9)
    );
  SPI_MOD_FIFO_THRESHOLD_8_SPI_MOD_FIFO_THRESHOLD_8_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_FIFO_THRESHOLD_7_pack_6,
      O => SPI_MOD_FIFO_THRESHOLD(7)
    );
  SPI_MOD_FIFO_THRESHOLD_8_SPI_MOD_FIFO_THRESHOLD_8_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_FIFO_THRESHOLD_5_pack_4,
      O => SPI_MOD_FIFO_THRESHOLD(5)
    );
  SPI_MOD_FIFO_THRESHOLD_5_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(5),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(5),
      O => SPI_MOD_FIFO_THRESHOLD_5_dpot_6837
    );
  SPI_MOD_FIFO_THRESHOLD_8 : X_FF
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_8_dpot1_4453,
      O => SPI_MOD_FIFO_THRESHOLD(8),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_8_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => X"F0F0FF00F0F0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_MOD_FIFO_THRESHOLD_8_dpot_5834,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(8),
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      ADR5 => '1',
      O => SPI_MOD_FIFO_THRESHOLD_8_dpot1_4453
    );
  SPI_MOD_FIFO_THRESHOLD_9_dpot1 : X_LUT5
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => X"CCCCAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(9),
      ADR1 => SPI_MOD_FIFO_THRESHOLD_9_dpot_5833,
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      O => SPI_MOD_FIFO_THRESHOLD_9_dpot1_4454
    );
  SPI_MOD_FIFO_THRESHOLD_9 : X_FF
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_9_dpot1_4454,
      O => SPI_MOD_FIFO_THRESHOLD_9_pack_8,
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_6 : X_FF
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_6_dpot1_4462,
      O => SPI_MOD_FIFO_THRESHOLD(6),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_6_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => X"F0F0FF00F0F0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_MOD_FIFO_THRESHOLD_6_dpot_5924,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(6),
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      ADR5 => '1',
      O => SPI_MOD_FIFO_THRESHOLD_6_dpot1_4462
    );
  SPI_MOD_FIFO_THRESHOLD_7_dpot1 : X_LUT5
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => X"CCCCAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(7),
      ADR1 => SPI_MOD_FIFO_THRESHOLD_7_dpot_5923,
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      O => SPI_MOD_FIFO_THRESHOLD_7_dpot1_4463
    );
  SPI_MOD_FIFO_THRESHOLD_7 : X_FF
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_7_dpot1_4463,
      O => SPI_MOD_FIFO_THRESHOLD_7_pack_6,
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_4 : X_FF
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_4_dpot1_4471,
      O => SPI_MOD_FIFO_THRESHOLD(4),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_4_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => X"F0F0FF00F0F0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_MOD_FIFO_THRESHOLD_4_dpot_5825,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(4),
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      ADR5 => '1',
      O => SPI_MOD_FIFO_THRESHOLD_4_dpot1_4471
    );
  SPI_MOD_FIFO_THRESHOLD_5_dpot1 : X_LUT5
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => X"CCCCAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(5),
      ADR1 => SPI_MOD_FIFO_THRESHOLD_5_dpot_6837,
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      O => SPI_MOD_FIFO_THRESHOLD_5_dpot1_4472
    );
  SPI_MOD_FIFO_THRESHOLD_5 : X_FF
    generic map(
      LOC => "SLICE_X14Y20",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_5_dpot1_4472,
      O => SPI_MOD_FIFO_THRESHOLD_5_pack_4,
      RST => GND,
      SET => GND
    );
  SPI_selected_register_2_SPI_selected_register_2_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => N6_pack_7,
      O => N6
    );
  SPI_selected_register_2_SPI_selected_register_2_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_Mcount_selected_register,
      O => SPI_Mcount_selected_register_0
    );
  SPI_selected_register_2 : X_FF
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0474_inv_5916,
      CLK => clk_pll,
      I => SPI_Mcount_selected_register2,
      O => SPI_selected_register(2),
      RST => GND,
      SET => GND
    );
  SPI_Mcount_selected_register_xor_2_11 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => X"7FD52A807FD52A80"
    )
    port map (
      ADR0 => SPI_state_FSM_FFd1_5903,
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(1),
      ADR3 => SPI_selected_register(2),
      ADR4 => SPI_SPI_BUF_OUT(2),
      ADR5 => '1',
      O => SPI_Mcount_selected_register2
    );
  SPI_Mcount_selected_register_xor_4_1_SW0 : X_LUT5
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => X"3F3F3F3F"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(1),
      ADR3 => '1',
      ADR4 => '1',
      O => N6_pack_7
    );
  SPI_selected_register_4 : X_FF
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0474_inv_5916,
      CLK => clk_pll,
      I => SPI_Mcount_selected_register4,
      O => SPI_selected_register(4),
      RST => GND,
      SET => GND
    );
  SPI_Mcount_selected_register_xor_4_1 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => X"FF557FD5AA002A80"
    )
    port map (
      ADR0 => SPI_state_FSM_FFd1_5903,
      ADR1 => SPI_selected_register(3),
      ADR2 => SPI_selected_register(2),
      ADR3 => SPI_selected_register(4),
      ADR4 => N6,
      ADR5 => SPI_SPI_BUF_OUT(4),
      O => SPI_Mcount_selected_register4
    );
  SPI_selected_register_1_1 : X_FF
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0474_inv_5916,
      CLK => clk_pll,
      I => SPI_Mcount_selected_register1,
      O => SPI_selected_register_1_1_6742,
      RST => GND,
      SET => GND
    );
  SPI_Mcount_selected_register_xor_1_11 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => X"3FF30CC03FF30CC0"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(1),
      ADR4 => SPI_SPI_BUF_OUT(1),
      ADR5 => '1',
      O => SPI_Mcount_selected_register1
    );
  SPI_Mcount_selected_register_xor_0_11 : X_LUT5
    generic map(
      LOC => "SLICE_X14Y21",
      INIT => X"2E2E2E2E"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(0),
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_selected_register(0),
      ADR3 => '1',
      ADR4 => '1',
      O => SPI_Mcount_selected_register
    );
  SPI_n0487_inv_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y22",
      INIT => X"FFFFFFF3FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_selected_register_0_1_6741,
      ADR2 => Inst_ResetDelay_OUT_RESET_2_6660,
      ADR3 => SPI_selected_register_1_1_6742,
      ADR4 => SPI_selected_register(4),
      ADR5 => SPI_SPI_COMPLETE_6402,
      O => N10
    );
  SPI_MOD_FIFO_DATA_3_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X14Y24"
    )
    port map (
      IA => N103,
      IB => N104,
      O => SPI_MOD_FIFO_DATA_3_dpot_4516,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_3_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X14Y24",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(3),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(11),
      O => N103
    );
  SPI_MOD_FIFO_DATA_3 : X_FF
    generic map(
      LOC => "SLICE_X14Y24",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_3_dpot_4516,
      O => SPI_MOD_FIFO_DATA(3),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_3_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X14Y24",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(3),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(11),
      O => N104
    );
  SPI_MOD_FIFO_DATA_1_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X14Y25"
    )
    port map (
      IA => N99,
      IB => N100,
      O => SPI_MOD_FIFO_DATA_1_dpot_4558,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_1_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X14Y25",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(1),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(9),
      O => N99
    );
  SPI_MOD_FIFO_DATA_1 : X_FF
    generic map(
      LOC => "SLICE_X14Y25",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_1_dpot_4558,
      O => SPI_MOD_FIFO_DATA(1),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_1_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X14Y25",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(1),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(9),
      O => N100
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o111 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y25",
      INIT => X"00000F0000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Modulation_write_pos(1),
      ADR3 => Modulation_read_pos(0),
      ADR4 => Modulation_write_pos(0),
      ADR5 => Modulation_read_pos(1),
      O => Modulation_read_pos_10_write_pos_10_equal_9_o11
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o113 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y25",
      INIT => X"000000F0F0000F00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Modulation_write_pos(1),
      ADR3 => Modulation_write_pos(0),
      ADR4 => Modulation_read_pos(1),
      ADR5 => Modulation_read_pos(0),
      O => Modulation_read_pos_10_write_pos_10_equal_9_o112_6700
    );
  SPI_MOD_FIFO_DATA_5_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X14Y26"
    )
    port map (
      IA => N107,
      IB => N108,
      O => SPI_MOD_FIFO_DATA_5_dpot_4564,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_5_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X14Y26",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(5),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(13),
      O => N107
    );
  SPI_MOD_FIFO_DATA_5 : X_FF
    generic map(
      LOC => "SLICE_X14Y26",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_5_dpot_4564,
      O => SPI_MOD_FIFO_DATA(5),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_5_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X14Y26",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(5),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(13),
      O => N108
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o61_Modulation_read_pos_10_write_pos_10_equal_9_o61_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Modulation_read_pos_10_write_pos_10_equal_9_o116_4583,
      O => Modulation_read_pos_10_write_pos_10_equal_9_o116_0
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o117 : X_MUX2
    generic map(
      LOC => "SLICE_X14Y27"
    )
    port map (
      IA => N95,
      IB => N96,
      O => Modulation_read_pos_10_write_pos_10_equal_9_o116_4583,
      SEL => Modulation_read_pos_10_write_pos_10_equal_9_o61
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o117_F : X_LUT6
    generic map(
      LOC => "SLICE_X14Y27",
      INIT => X"0006009090000900"
    )
    port map (
      ADR0 => Modulation_write_pos(8),
      ADR1 => Modulation_read_pos(8),
      ADR2 => Modulation_write_pos(7),
      ADR3 => Modulation_write_pos(6),
      ADR4 => Modulation_read_pos(7),
      ADR5 => Modulation_read_pos(6),
      O => N95
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o117_G : X_LUT6
    generic map(
      LOC => "SLICE_X14Y27",
      INIT => X"9009000000009009"
    )
    port map (
      ADR0 => Modulation_write_pos(7),
      ADR1 => Modulation_read_pos(7),
      ADR2 => Modulation_write_pos(8),
      ADR3 => Modulation_read_pos(8),
      ADR4 => Modulation_write_pos(6),
      ADR5 => Modulation_read_pos(6),
      O => N96
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o611 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y27",
      INIT => X"7FFFFFFFFFFFFFFF"
    )
    port map (
      ADR0 => Modulation_read_pos(4),
      ADR1 => Modulation_read_pos(1),
      ADR2 => Modulation_read_pos(0),
      ADR3 => Modulation_read_pos(3),
      ADR4 => Modulation_read_pos(2),
      ADR5 => Modulation_read_pos(5),
      O => Modulation_read_pos_10_write_pos_10_equal_9_o61
    );
  Modulation_read_pos_7_rstpot_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y27",
      INIT => X"CFCFC0C0CFCFC840"
    )
    port map (
      ADR0 => Modulation_read_pos(6),
      ADR1 => Modulation_read_pos(7),
      ADR2 => Modulation_read_pos_10_write_pos_10_equal_9_o117_6703,
      ADR3 => N24,
      ADR4 => Modulation_Result_7_1_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o61,
      O => N77
    );
  SPI_MOD_FIFO_DATA_0_dpot : X_MUX2
    generic map(
      LOC => "SLICE_X14Y28"
    )
    port map (
      IA => N97,
      IB => N98,
      O => SPI_MOD_FIFO_DATA_0_dpot_4613,
      SEL => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_DATA_0_dpot_F : X_LUT6
    generic map(
      LOC => "SLICE_X14Y28",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_DATA(0),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(8),
      O => N97
    );
  SPI_MOD_FIFO_DATA_0 : X_FF
    generic map(
      LOC => "SLICE_X14Y28",
      INIT => '0'
    )
    port map (
      CE => SPI_n0440_inv1_cepot,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_DATA_0_dpot_4613,
      O => SPI_MOD_FIFO_DATA(0),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_DATA_0_dpot_G : X_LUT6
    generic map(
      LOC => "SLICE_X14Y28",
      INIT => X"AABAAAAAAA8AAAAA"
    )
    port map (
      ADR0 => SPI_mod_data_LSB(0),
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(8),
      O => N98
    );
  SPI_n0440_inv1_cepot_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y28",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_n0440_inv1_cepot
    );
  SPI_word_cnt_4_SPI_word_cnt_4_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_word_cnt_3_pack_13,
      O => SPI_word_cnt(3)
    );
  SPI_word_cnt_4_SPI_word_cnt_4_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_word_cnt_1_pack_11,
      O => SPI_word_cnt(1)
    );
  SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_1 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => X"0000003000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_word_cnt(4),
      ADR2 => SPI_word_cnt(3),
      ADR3 => SPI_word_cnt(2),
      ADR4 => SPI_word_cnt(1),
      ADR5 => SPI_word_cnt(0),
      O => SPI_GND_17_o_word_cnt_4_equal_23_o_4_1_6694
    );
  SPI_word_cnt_4 : X_SFF
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0652_inv,
      CLK => clk_pll,
      I => SPI_Result(4),
      O => SPI_word_cnt(4),
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_Mcount_word_cnt_xor_4_11 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => X"7F80FF007F80FF00"
    )
    port map (
      ADR0 => SPI_word_cnt(0),
      ADR1 => SPI_word_cnt(1),
      ADR2 => SPI_word_cnt(2),
      ADR3 => SPI_word_cnt(4),
      ADR4 => SPI_word_cnt(3),
      ADR5 => '1',
      O => SPI_Result(4)
    );
  SPI_Mcount_word_cnt_xor_3_11 : X_LUT5
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => X"7F7F8080"
    )
    port map (
      ADR0 => SPI_word_cnt(0),
      ADR1 => SPI_word_cnt(1),
      ADR2 => SPI_word_cnt(2),
      ADR3 => '1',
      ADR4 => SPI_word_cnt(3),
      O => SPI_Result(3)
    );
  SPI_word_cnt_3 : X_SFF
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0652_inv,
      CLK => clk_pll,
      I => SPI_Result(3),
      O => SPI_word_cnt_3_pack_13,
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_word_cnt_2 : X_SFF
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0652_inv,
      CLK => clk_pll,
      I => SPI_Result(2),
      O => SPI_word_cnt(2),
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_Mcount_word_cnt_xor_2_11 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => X"0FF0FF000FF0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_word_cnt(0),
      ADR3 => SPI_word_cnt(2),
      ADR4 => SPI_word_cnt(1),
      ADR5 => '1',
      O => SPI_Result(2)
    );
  SPI_Mcount_word_cnt_xor_1_11 : X_LUT5
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => X"0F0FF0F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_word_cnt(0),
      ADR3 => '1',
      ADR4 => SPI_word_cnt(1),
      O => SPI_Result(1)
    );
  SPI_word_cnt_1 : X_SFF
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0652_inv,
      CLK => clk_pll,
      I => SPI_Result(1),
      O => SPI_word_cnt_1_pack_11,
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_word_cnt_0 : X_SFF
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => '0'
    )
    port map (
      CE => SPI_n0652_inv,
      CLK => clk_pll,
      I => SPI_Result(0),
      O => SPI_word_cnt(0),
      SRST => SPI_n0394,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_Mcount_word_cnt_xor_0_11_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X14Y29",
      INIT => X"00FF00FF00FF00FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_word_cnt(0),
      ADR4 => '1',
      ADR5 => '1',
      O => SPI_Result(0)
    );
  INV_SPI_SPI_miso_buffer_6CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_Mcount_bit_cnt2_INV_SPI_SPI_miso_buffer_6CLK
    );
  SPI_SPI_Mcount_bit_cnt2_SPI_SPI_Mcount_bit_cnt2_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_SPI_miso_buffer(6),
      O => SPI_SPI_miso_buffer_6_0
    );
  SPI_SPI_Mcount_bit_cnt21 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y7",
      INIT => X"0FF0F0F00FF0F0F0"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(0),
      ADR4 => SPI_SPI_bit_cnt(1),
      ADR5 => '1',
      O => SPI_SPI_Mcount_bit_cnt2
    );
  SPI_SPI_mux1211 : X_LUT5
    generic map(
      LOC => "SLICE_X15Y7",
      INIT => X"AAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(5),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(0),
      ADR4 => SPI_SPI_bit_cnt(1),
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_6_Q
    );
  SPI_SPI_miso_buffer_6 : X_FF
    generic map(
      LOC => "SLICE_X15Y7",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_Mcount_bit_cnt2_INV_SPI_SPI_miso_buffer_6CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_6_Q,
      O => SPI_SPI_miso_buffer(6),
      RST => GND,
      SET => GND
    );
  INV_SPI_SPI_miso_buffer_3CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_3CLK
    );
  INV_SPI_SPI_miso_buffer_2CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_2CLK
    );
  INV_SPI_SPI_miso_buffer_1CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_1CLK
    );
  INV_SPI_SPI_miso_buffer_0CLK : X_INV
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => MCU_SCK_IBUF_BUFG_5814,
      O => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_0CLK
    );
  SPI_SPI_miso_buffer_3 : X_FF
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_3CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_3_Q,
      O => SPI_SPI_miso_buffer(3),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux911 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(2),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_3_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_3_Q
    );
  SPI_SPI_miso_buffer_2 : X_FF
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_2CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_2_Q,
      O => SPI_SPI_miso_buffer(2),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux811 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(1),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_2_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_2_Q
    );
  SPI_SPI_miso_buffer_1 : X_FF
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_1CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_1_Q,
      O => SPI_SPI_miso_buffer(1),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux711 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => X"AAAAAAABAAAAAAA8"
    )
    port map (
      ADR0 => SPI_SPI_miso_buffer(0),
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_1_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_1_Q
    );
  SPI_SPI_miso_buffer_0 : X_FF
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => '0'
    )
    port map (
      CE => SPI_SPI_n0066_inv,
      CLK => SPI_SPI_miso_buffer_3_INV_SPI_SPI_miso_buffer_0CLK,
      I => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_0_Q,
      O => SPI_SPI_miso_buffer(0),
      RST => GND,
      SET => GND
    );
  SPI_SPI_mux161 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y12",
      INIT => X"0000000300000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_SPI_bit_cnt(3),
      ADR2 => SPI_SPI_bit_cnt(2),
      ADR3 => SPI_SPI_bit_cnt(1),
      ADR4 => SPI_SPI_bit_cnt(0),
      ADR5 => SPI_spi_buf_in_15_Q,
      O => SPI_SPI_miso_buffer_14_BUF_IN_15_mux_12_OUT_0_Q
    );
  SPI_GND_17_o_INV_43_o_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y13",
      INIT => X"FFFFF000F000F000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_interrupt_status(3),
      ADR3 => SPI_interrupt_mask(3),
      ADR4 => SPI_interrupt_status(4),
      ADR5 => SPI_interrupt_mask(4),
      O => N16
    );
  SPI_INTERRUPT_ASSERTED : X_SFF
    generic map(
      LOC => "SLICE_X15Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_GND_17_o_INV_43_o_4716,
      O => SPI_INTERRUPT_ASSERTED_6389,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_GND_17_o_INV_43_o : X_LUT6
    generic map(
      LOC => "SLICE_X15Y13",
      INIT => X"FFFFFFC0FFC0FFC0"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_interrupt_status(2),
      ADR2 => SPI_interrupt_mask(2),
      ADR3 => N16,
      ADR4 => SPI_interrupt_status(1),
      ADR5 => SPI_interrupt_mask(1),
      O => SPI_GND_17_o_INV_43_o_4716
    );
  SPI_interrupt_mask_3 : X_SFF
    generic map(
      LOC => "SLICE_X15Y14",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_1_cepot_0,
      CLK => clk_pll,
      I => SPI_interrupt_mask_3_dpot1_4724,
      O => SPI_interrupt_mask(3),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_interrupt_mask_3_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y14",
      INIT => X"FF00FE02FF00FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(3),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_interrupt_mask(3),
      ADR4 => SPI_selected_register(2),
      ADR5 => SPI_n0539_inv11_1_rstpot_6425,
      O => SPI_interrupt_mask_3_dpot1_4724
    );
  SPI_interrupt_mask_2 : X_SFF
    generic map(
      LOC => "SLICE_X15Y14",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_1_cepot_0,
      CLK => clk_pll,
      I => SPI_interrupt_mask_2_dpot1_4734,
      O => SPI_interrupt_mask(2),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_interrupt_mask_2_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y14",
      INIT => X"FF00FE02FF00FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(2),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_interrupt_mask(2),
      ADR4 => SPI_selected_register(2),
      ADR5 => SPI_n0539_inv11_1_rstpot_6425,
      O => SPI_interrupt_mask_2_dpot1_4734
    );
  SPI_n0539_inv11_1_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y14",
      INIT => X"0300000000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_selected_register(4),
      ADR2 => SPI_selected_register(3),
      ADR3 => SPI_SPI_COMPLETE_6402,
      ADR4 => SPI_state_FSM_FFd2_6400,
      ADR5 => SPI_state_FSM_FFd1_5903,
      O => SPI_n0539_inv11_1_rstpot_6425
    );
  SPI_interrupt_mask_1 : X_SFF
    generic map(
      LOC => "SLICE_X15Y14",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_1_cepot_0,
      CLK => clk_pll,
      I => SPI_interrupt_mask_1_dpot1_4748,
      O => SPI_interrupt_mask(1),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_interrupt_mask_1_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y14",
      INIT => X"FF00FE02FF00FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(1),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_interrupt_mask(1),
      ADR4 => SPI_selected_register(2),
      ADR5 => SPI_n0539_inv11_1_rstpot_6425,
      O => SPI_interrupt_mask_1_dpot1_4748
    );
  SPI_MOD_FIFO_THRESHOLD_2_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y19",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(2),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(2),
      O => SPI_MOD_FIFO_THRESHOLD_2_dpot_5920
    );
  SPI_selected_register_3 : X_FF
    generic map(
      LOC => "SLICE_X15Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0474_inv_5916,
      CLK => clk_pll,
      I => SPI_Mcount_selected_register3,
      O => SPI_selected_register(3),
      RST => GND,
      SET => GND
    );
  SPI_Mcount_selected_register_xor_3_11 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y19",
      INIT => X"7FD5FF552A80AA00"
    )
    port map (
      ADR0 => SPI_state_FSM_FFd1_5903,
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(1),
      ADR3 => SPI_selected_register(3),
      ADR4 => SPI_selected_register(2),
      ADR5 => SPI_SPI_BUF_OUT(3),
      O => SPI_Mcount_selected_register3
    );
  SPI_selected_register_1 : X_FF
    generic map(
      LOC => "SLICE_X15Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0474_inv_5916,
      CLK => clk_pll,
      I => SPI_Mcount_selected_register1,
      O => SPI_selected_register(1),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_1_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y19",
      INIT => X"AAAAAABAAAAAAA8A"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(1),
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(2),
      ADR3 => SPI_selected_register(1),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(1),
      O => SPI_MOD_FIFO_THRESHOLD_1_dpot_5876
    );
  SPI_selected_register_0 : X_FF
    generic map(
      LOC => "SLICE_X15Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0474_inv_5916,
      CLK => clk_pll,
      I => SPI_Mcount_selected_register_0,
      O => SPI_selected_register(0),
      RST => GND,
      SET => GND
    );
  SPI_n0484_inv_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y19",
      INIT => X"FFFFF0FFFFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_selected_register(4),
      ADR3 => SPI_selected_register(0),
      ADR4 => SPI_selected_register(1),
      ADR5 => SPI_SPI_COMPLETE_6402,
      O => N8
    );
  SPI_selected_register_0_1 : X_FF
    generic map(
      LOC => "SLICE_X15Y21",
      INIT => '0'
    )
    port map (
      CE => SPI_n0474_inv_5916,
      CLK => clk_pll,
      I => SPI_Mcount_selected_register_0,
      O => SPI_selected_register_0_1_6741,
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_7_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y21",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(7),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(7),
      O => SPI_MOD_FIFO_THRESHOLD_7_dpot_5923
    );
  SPI_MOD_FIFO_THRESHOLD_9_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y21",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(9),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(9),
      O => SPI_MOD_FIFO_THRESHOLD_9_dpot_5833
    );
  SPI_mod_data_LSB_3 : X_FF
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_3_dpot_4816,
      O => SPI_mod_data_LSB(3),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_3_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(3),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(3),
      O => SPI_mod_data_LSB_3_dpot_4816
    );
  SPI_mod_data_LSB_2 : X_FF
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_2_dpot_4826,
      O => SPI_mod_data_LSB(2),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_2_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(2),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(2),
      O => SPI_mod_data_LSB_2_dpot_4826
    );
  SPI_mod_data_LSB_1 : X_FF
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_1_dpot_4834,
      O => SPI_mod_data_LSB(1),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_1_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(1),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(1),
      O => SPI_mod_data_LSB_1_dpot_4834
    );
  SPI_mod_data_LSB_0 : X_FF
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => '0'
    )
    port map (
      CE => SPI_n0588_inv1_cepot_0,
      CLK => clk_pll,
      I => SPI_mod_data_LSB_0_dpot_4809,
      O => SPI_mod_data_LSB(0),
      RST => GND,
      SET => GND
    );
  SPI_mod_data_LSB_0_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y25",
      INIT => X"FF04FF00FB00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => SPI_state_FSM_FFd1_5903,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_mod_data_LSB(0),
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => SPI_SPI_BUF_OUT(0),
      O => SPI_mod_data_LSB_0_dpot_4809
    );
  SPI_LEDS_2 : X_SFF
    generic map(
      LOC => "SLICE_X15Y26",
      INIT => '1'
    )
    port map (
      CE => SPI_n0539_inv11_1_cepot_0,
      CLK => clk_pll,
      I => SPI_LEDS_2_dpot1_4858,
      O => SPI_LEDS(2),
      SSET => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  SPI_LEDS_2_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y26",
      INIT => X"FD01FF00FF00FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(15),
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(2),
      ADR3 => SPI_LEDS(2),
      ADR4 => SPI_selected_register(1),
      ADR5 => SPI_n0539_inv11_1_rstpot_6425,
      O => SPI_LEDS_2_dpot1_4858
    );
  SPI_LEDS_1 : X_SFF
    generic map(
      LOC => "SLICE_X15Y26",
      INIT => '1'
    )
    port map (
      CE => SPI_n0539_inv11_1_cepot_0,
      CLK => clk_pll,
      I => SPI_LEDS_1_dpot1_4866,
      O => SPI_LEDS(1),
      SSET => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  SPI_LEDS_1_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y26",
      INIT => X"FD01FF00FF00FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(14),
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(2),
      ADR3 => SPI_LEDS(1),
      ADR4 => SPI_selected_register(1),
      ADR5 => SPI_n0539_inv11_1_rstpot_6425,
      O => SPI_LEDS_1_dpot1_4866
    );
  SPI_LEDS_0 : X_SFF
    generic map(
      LOC => "SLICE_X15Y26",
      INIT => '1'
    )
    port map (
      CE => SPI_n0539_inv11_1_cepot_0,
      CLK => clk_pll,
      I => SPI_LEDS_0_dpot1_4847,
      O => SPI_LEDS(0),
      SSET => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  SPI_LEDS_0_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y26",
      INIT => X"FD01FF00FF00FF00"
    )
    port map (
      ADR0 => SPI_SPI_BUF_OUT(13),
      ADR1 => SPI_selected_register(0),
      ADR2 => SPI_selected_register(2),
      ADR3 => SPI_LEDS(0),
      ADR4 => SPI_selected_register(1),
      ADR5 => SPI_n0539_inv11_1_rstpot_6425,
      O => SPI_LEDS_0_dpot1_4847
    );
  SPI_n0652_inv1 : X_LUT6
    generic map(
      LOC => "SLICE_X15Y29",
      INIT => X"00000000FFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_SPI_COMPLETE_6402,
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => SPI_n0652_inv
    );
  SPI_MOD_LOOKUP_DATA_39 : X_FF
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_39_dpot_4881,
      O => SPI_MOD_LOOKUP_DATA(39),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_39_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_23_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(39),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_39_dpot_4881
    );
  SPI_MOD_LOOKUP_DATA_38 : X_FF
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_38_dpot_4890,
      O => SPI_MOD_LOOKUP_DATA(38),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_38_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_22_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(38),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_38_dpot_4890
    );
  SPI_MOD_LOOKUP_DATA_37 : X_FF
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_37_dpot_4897,
      O => SPI_MOD_LOOKUP_DATA(37),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_37_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_21_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(37),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_37_dpot_4897
    );
  SPI_MOD_LOOKUP_DATA_36 : X_FF
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_36_dpot_4874,
      O => SPI_MOD_LOOKUP_DATA(36),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_36_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_20_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(36),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_36_dpot_4874
    );
  SPI_MOD_LOOKUP_DATA_67 : X_FF
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_67_dpot_4911,
      O => SPI_MOD_LOOKUP_DATA(67),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_67_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(51),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(67),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_67_dpot_4911
    );
  SPI_MOD_LOOKUP_DATA_66 : X_FF
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_66_dpot_4920,
      O => SPI_MOD_LOOKUP_DATA(66),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_66_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(50),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(66),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_66_dpot_4920
    );
  SPI_MOD_LOOKUP_DATA_65 : X_FF
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_65_dpot_4927,
      O => SPI_MOD_LOOKUP_DATA(65),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_65_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(49),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(65),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_65_dpot_4927
    );
  SPI_MOD_LOOKUP_DATA_64 : X_FF
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_64_dpot_4904,
      O => SPI_MOD_LOOKUP_DATA(64),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_64_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X15Y38",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(48),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(64),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_64_dpot_4904
    );
  Sync_AUX3_sync_line_2 : X_FF
    generic map(
      LOC => "SLICE_X16Y10",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_AUX3_Mshreg_sync_line_2_4934,
      O => Sync_AUX3_sync_line(2),
      RST => GND,
      SET => GND
    );
  Sync_AUX3_Mshreg_sync_line_2 : X_SRLC16E
    generic map(
      LOC => "SLICE_X16Y10",
      INIT => X"0000"
    )
    port map (
      A0 => '1',
      A1 => '0',
      A2 => '0',
      A3 => '0',
      CLK => clk_pll,
      D => MCU_AUX3_IBUF_0,
      Q15 => NLW_Sync_AUX3_Mshreg_sync_line_2_Q15_UNCONNECTED,
      Q => Sync_AUX3_Mshreg_sync_line_2_4934,
      CE => '1'
    );
  Sync_SOURCE_LD_sync_line_2 : X_FF
    generic map(
      LOC => "SLICE_X16Y10",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Sync_SOURCE_LD_Mshreg_sync_line_2_4941,
      O => Sync_SOURCE_LD_sync_line(2),
      RST => GND,
      SET => GND
    );
  Sync_SOURCE_LD_Mshreg_sync_line_2 : X_SRLC16E
    generic map(
      LOC => "SLICE_X16Y10",
      INIT => X"0000"
    )
    port map (
      A0 => '1',
      A1 => '0',
      A2 => '0',
      A3 => '0',
      CLK => clk_pll,
      D => LEDS_2_OBUF_0,
      Q15 => NLW_Sync_SOURCE_LD_Mshreg_sync_line_2_Q15_UNCONNECTED,
      Q => Sync_SOURCE_LD_Mshreg_sync_line_2_4941,
      CE => '1'
    );
  SPI_interrupt_status_4 : X_SFF
    generic map(
      LOC => "SLICE_X16Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_THRESHOLD_CROSSED_6320,
      O => SPI_interrupt_status(4),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_interrupt_status_3 : X_SFF
    generic map(
      LOC => "SLICE_X16Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_UNDERFLOW_6687,
      O => SPI_interrupt_status(3),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_interrupt_status_2 : X_SFF
    generic map(
      LOC => "SLICE_X16Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_OVERFLOW_6686,
      O => SPI_interrupt_status(2),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_interrupt_status_1 : X_SFF
    generic map(
      LOC => "SLICE_X16Y13",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => source_unlocked,
      O => SPI_interrupt_status(1),
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  source_unlocked1_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y13",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Sync_SOURCE_LD_sync_line(2),
      O => source_unlocked
    );
  SPI_MOD_FIFO_THRESHOLD_2_SPI_MOD_FIFO_THRESHOLD_2_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_FIFO_THRESHOLD_3_pack_7,
      O => SPI_MOD_FIFO_THRESHOLD(3)
    );
  SPI_MOD_FIFO_THRESHOLD_2_SPI_MOD_FIFO_THRESHOLD_2_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_MOD_FIFO_THRESHOLD_10_pack_5,
      O => SPI_MOD_FIFO_THRESHOLD(10)
    );
  SPI_MOD_FIFO_THRESHOLD_2 : X_FF
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_2_dpot1_4962,
      O => SPI_MOD_FIFO_THRESHOLD(2),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_2_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => X"F0F0FF00F0F0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_MOD_FIFO_THRESHOLD_2_dpot_5920,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(2),
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      ADR5 => '1',
      O => SPI_MOD_FIFO_THRESHOLD_2_dpot1_4962
    );
  SPI_MOD_FIFO_THRESHOLD_3_dpot1 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => X"CCCCAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(3),
      ADR1 => SPI_MOD_FIFO_THRESHOLD_3_dpot_5919,
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      O => SPI_MOD_FIFO_THRESHOLD_3_dpot1_4963
    );
  SPI_MOD_FIFO_THRESHOLD_3 : X_FF
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_3_dpot1_4963,
      O => SPI_MOD_FIFO_THRESHOLD_3_pack_7,
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_1 : X_FF
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_1_dpot1_4981,
      O => SPI_MOD_FIFO_THRESHOLD(1),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_1_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => X"F0F0FF00F0F0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => SPI_MOD_FIFO_THRESHOLD_1_dpot_5876,
      ADR3 => SPI_MOD_FIFO_THRESHOLD(1),
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      ADR5 => '1',
      O => SPI_MOD_FIFO_THRESHOLD_1_dpot1_4981
    );
  SPI_MOD_FIFO_THRESHOLD_10_dpot1 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => X"CCCCAAAA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(10),
      ADR1 => SPI_MOD_FIFO_THRESHOLD_10_dpot_5875,
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_n0539_inv11_2_rstpot_5827,
      O => SPI_MOD_FIFO_THRESHOLD_10_dpot1_4982
    );
  SPI_MOD_FIFO_THRESHOLD_10 : X_FF
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_10_dpot1_4982,
      O => SPI_MOD_FIFO_THRESHOLD_10_pack_5,
      RST => GND,
      SET => GND
    );
  SPI_n0539_inv11_2_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => X"0300000000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_selected_register(4),
      ADR2 => SPI_selected_register(3),
      ADR3 => SPI_SPI_COMPLETE_6402,
      ADR4 => SPI_state_FSM_FFd2_6400,
      ADR5 => SPI_state_FSM_FFd1_5903,
      O => SPI_n0539_inv11_2_rstpot_5827
    );
  SPI_MOD_FIFO_THRESHOLD_0 : X_FF
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => '0'
    )
    port map (
      CE => SPI_n0539_inv11_2_cepot_5823,
      CLK => clk_pll,
      I => SPI_MOD_FIFO_THRESHOLD_0_dpot1_4968,
      O => SPI_MOD_FIFO_THRESHOLD(0),
      RST => GND,
      SET => GND
    );
  SPI_MOD_FIFO_THRESHOLD_0_dpot1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y19",
      INIT => X"FFFF0000FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_MOD_FIFO_THRESHOLD(0),
      ADR4 => SPI_MOD_FIFO_THRESHOLD_0_dpot_6690,
      ADR5 => SPI_n0539_inv11_2_rstpot_5827,
      O => SPI_MOD_FIFO_THRESHOLD_0_dpot1_4968
    );
  SPI_mod_table_write_SPI_mod_table_write_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_first_byte_pack_2,
      O => SPI_mod_first_byte_6405
    );
  SPI_MOD_FIFO_WRITE1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y29",
      INIT => X"FFFFFFFFFFFF0000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => SPI_mod_second_byte_6429,
      ADR5 => SPI_mod_first_byte_6405,
      O => mod_sample_latch
    );
  Modulation_n0068_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y29",
      INIT => X"8421000000000000"
    )
    port map (
      ADR0 => Modulation_write_pos(0),
      ADR1 => Modulation_write_pos(1),
      ADR2 => Modulation_read_pos(0),
      ADR3 => Modulation_read_pos(1),
      ADR4 => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_Q_6731,
      ADR5 => mod_sample_latch,
      O => N22
    );
  SPI_mod_table_write : X_SFF
    generic map(
      LOC => "SLICE_X16Y29",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_mod_table_write_glue_set,
      O => SPI_mod_table_write_5972,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_n04081 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y29",
      INIT => X"0008000000080000"
    )
    port map (
      ADR0 => SPI_SPI_COMPLETE_6402,
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      ADR5 => '1',
      O => SPI_mod_table_write_glue_set
    );
  SPI_n0400_1_1 : X_LUT5
    generic map(
      LOC => "SLICE_X16Y29",
      INIT => X"00200020"
    )
    port map (
      ADR0 => SPI_SPI_COMPLETE_6402,
      ADR1 => SPI_state_FSM_FFd2_6400,
      ADR2 => SPI_state_FSM_FFd1_5903,
      ADR3 => SPI_NSS_last_NSS_AND_33_o,
      ADR4 => '1',
      O => SPI_mod_first_byte_glue_set
    );
  SPI_mod_first_byte : X_SFF
    generic map(
      LOC => "SLICE_X16Y29",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_mod_first_byte_glue_set,
      O => SPI_mod_first_byte_pack_2,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_GND_17_o_word_cnt_4_equal_23_o_4_1 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y29",
      INIT => X"0000003000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_word_cnt(4),
      ADR2 => SPI_word_cnt(3),
      ADR3 => SPI_word_cnt(2),
      ADR4 => SPI_word_cnt(1),
      ADR5 => SPI_word_cnt(0),
      O => SPI_GND_17_o_word_cnt_4_equal_23_o
    );
  SPI_MOD_LOOKUP_DATA_43 : X_FF
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_43_dpot_5033,
      O => SPI_MOD_LOOKUP_DATA(43),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_43_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_27_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(43),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_43_dpot_5033
    );
  SPI_MOD_LOOKUP_DATA_42 : X_FF
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_42_dpot_5042,
      O => SPI_MOD_LOOKUP_DATA(42),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_42_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_26_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(42),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_42_dpot_5042
    );
  SPI_MOD_LOOKUP_DATA_41 : X_FF
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_41_dpot_5049,
      O => SPI_MOD_LOOKUP_DATA(41),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_41_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_25_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(41),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_41_dpot_5049
    );
  SPI_MOD_LOOKUP_DATA_40 : X_FF
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_40_dpot_5026,
      O => SPI_MOD_LOOKUP_DATA(40),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_40_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y34",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_24_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(40),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_40_dpot_5026
    );
  SPI_MOD_LOOKUP_DATA_55 : X_FF
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_55_dpot_5063,
      O => SPI_MOD_LOOKUP_DATA(55),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_55_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(39),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(55),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_55_dpot_5063
    );
  SPI_MOD_LOOKUP_DATA_54 : X_FF
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_54_dpot_5072,
      O => SPI_MOD_LOOKUP_DATA(54),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_54_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(38),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(54),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_54_dpot_5072
    );
  SPI_MOD_LOOKUP_DATA_53 : X_FF
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_53_dpot_5079,
      O => SPI_MOD_LOOKUP_DATA(53),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_53_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(37),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(53),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_53_dpot_5079
    );
  SPI_MOD_LOOKUP_DATA_52 : X_FF
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_111,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_52_dpot_5056,
      O => SPI_MOD_LOOKUP_DATA(52),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_52_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y36",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer(36),
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(52),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_52_dpot_5056
    );
  Source_latched_regs_57 : X_FF
    generic map(
      LOC => "SLICE_X16Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_57_dpot_5083,
      O => Source_latched_regs(57),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_57_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(56),
      ADR2 => mod_lookup_read_data(57),
      ADR3 => Source_latched_regs(57),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_57_dpot_5083
    );
  Source_n0148_inv11_rstpot_3 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y37",
      INIT => X"F0F0F0F00000CC00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_sclk_5839,
      ADR2 => source_reload_5964,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => Source_done_int_5965,
      O => Source_n0148_inv11_rstpot3
    );
  Source_latched_regs_56 : X_FF
    generic map(
      LOC => "SLICE_X16Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_56_dpot_5098,
      O => Source_latched_regs(56),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_56_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(55),
      ADR2 => mod_lookup_read_data(56),
      ADR3 => Source_latched_regs(56),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_56_dpot_5098
    );
  Source_latched_regs_55 : X_FF
    generic map(
      LOC => "SLICE_X16Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_55_dpot_5105,
      O => Source_latched_regs(55),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_55_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(54),
      ADR2 => mod_lookup_read_data(55),
      ADR3 => Source_latched_regs(55),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_55_dpot_5105
    );
  Source_n0148_inv11_cepot_1_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X16Y38",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => Inst_ResetDelay_OUT_RESET_5868,
      O => Source_n0148_inv11_cepot1
    );
  Source_latched_regs_63 : X_FF
    generic map(
      LOC => "SLICE_X16Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_63_dpot_5127,
      O => Source_latched_regs(63),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_63_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(62),
      ADR2 => mod_lookup_read_data(63),
      ADR3 => Source_latched_regs(63),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_63_dpot_5127
    );
  Source_latched_regs_62 : X_FF
    generic map(
      LOC => "SLICE_X16Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_62_dpot_5116,
      O => Source_latched_regs(62),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_62_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X16Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(61),
      ADR2 => mod_lookup_read_data(62),
      ADR3 => Source_latched_regs(62),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_62_dpot_5116
    );
  SPI_MOD_FIFO_THRESHOLD_3_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y19",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(3),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(3),
      O => SPI_MOD_FIFO_THRESHOLD_3_dpot_5919
    );
  SPI_MOD_FIFO_THRESHOLD_10_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y21",
      INIT => X"AAAAABAAAAAAA8AA"
    )
    port map (
      ADR0 => SPI_MOD_FIFO_THRESHOLD(10),
      ADR1 => SPI_selected_register(1),
      ADR2 => SPI_selected_register(0),
      ADR3 => SPI_selected_register(2),
      ADR4 => Inst_ResetDelay_OUT_RESET_5868,
      ADR5 => SPI_SPI_BUF_OUT(10),
      O => SPI_MOD_FIFO_THRESHOLD_10_dpot_5875
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_Q : X_LUT6
    generic map(
      LOC => "SLICE_X17Y24",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(4),
      ADR5 => Modulation_read_pos(4),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_Q_6699
    );
  Modulation_n006821 : X_LUT6
    generic map(
      LOC => "SLICE_X17Y24",
      INIT => X"8421000000000000"
    )
    port map (
      ADR0 => Modulation_write_pos(2),
      ADR1 => Modulation_write_pos(5),
      ADR2 => Modulation_read_pos(2),
      ADR3 => Modulation_read_pos(5),
      ADR4 => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_Q_6730,
      ADR5 => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_Q_6699,
      O => Modulation_n00682
    );
  Modulation_OVERFLOW_glue_set_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X17Y24",
      INIT => X"8421000000000000"
    )
    port map (
      ADR0 => Modulation_write_pos(8),
      ADR1 => Modulation_write_pos(9),
      ADR2 => Modulation_read_pos(8),
      ADR3 => Modulation_read_pos(9),
      ADR4 => Modulation_n00682,
      ADR5 => N22,
      O => N93
    );
  Modulation_OVERFLOW : X_SFF
    generic map(
      LOC => "SLICE_X17Y24",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_OVERFLOW_glue_set_5154,
      O => Modulation_OVERFLOW_6686,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_OVERFLOW_glue_set : X_LUT6
    generic map(
      LOC => "SLICE_X17Y24",
      INIT => X"FF82FF41FF00FF00"
    )
    port map (
      ADR0 => Modulation_write_pos(7),
      ADR1 => Modulation_write_pos(10),
      ADR2 => Modulation_read_pos(10),
      ADR3 => Modulation_OVERFLOW_6686,
      ADR4 => Modulation_read_pos(7),
      ADR5 => N93,
      O => Modulation_OVERFLOW_glue_set_5154
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_Q : X_LUT6
    generic map(
      LOC => "SLICE_X17Y25",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(5),
      ADR5 => Modulation_read_pos(5),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_Q_6848
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o112 : X_LUT6
    generic map(
      LOC => "SLICE_X17Y25",
      INIT => X"0802000000000000"
    )
    port map (
      ADR0 => Modulation_write_pos(2),
      ADR1 => Modulation_write_pos(3),
      ADR2 => Modulation_read_pos(2),
      ADR3 => Modulation_read_pos(3),
      ADR4 => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_5_Q_6848,
      ADR5 => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_4_Q_6699,
      O => Modulation_read_pos_10_write_pos_10_equal_9_o111_6847
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o116 : X_LUT6
    generic map(
      LOC => "SLICE_X17Y25",
      INIT => X"FCFCF8F0CCCC8800"
    )
    port map (
      ADR0 => Modulation_read_pos_10_write_pos_10_equal_9_o114_6702,
      ADR1 => Modulation_read_pos_10_write_pos_10_equal_9_o11,
      ADR2 => Modulation_read_pos_10_write_pos_10_equal_9_o112_6700,
      ADR3 => Modulation_read_pos_10_write_pos_10_equal_9_o113_6701,
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o111_6847,
      ADR5 => Modulation_n00682,
      O => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492
    );
  Modulation_UNDERFLOW : X_SFF
    generic map(
      LOC => "SLICE_X17Y25",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_UNDERFLOW_glue_set_5170,
      O => Modulation_UNDERFLOW_6687,
      SRST => mod_reset,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_UNDERFLOW_glue_set : X_LUT6
    generic map(
      LOC => "SLICE_X17Y25",
      INIT => X"B3A0330033003300"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => mod_sample_latch,
      ADR2 => N26,
      ADR3 => Modulation_UNDERFLOW_6687,
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_UNDERFLOW_glue_set_5170
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o1110_SW1 : X_LUT6
    generic map(
      LOC => "SLICE_X17Y26",
      INIT => X"8421842112848421"
    )
    port map (
      ADR0 => Modulation_write_pos(10),
      ADR1 => Modulation_write_pos(9),
      ADR2 => Modulation_read_pos(10),
      ADR3 => Modulation_read_pos(9),
      ADR4 => N83,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o61,
      O => N26
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o811_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X17Y26",
      INIT => X"FF00000000000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Modulation_read_pos(8),
      ADR4 => Modulation_read_pos(6),
      ADR5 => Modulation_read_pos(7),
      O => N83
    );
  Modulation_read_pos_7 : X_SFF
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_7_rstpot_5210,
      O => Modulation_read_pos(7),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_7_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => X"F5A0DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_7_1_0,
      ADR2 => N77,
      ADR3 => Modulation_read_pos(7),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_7_rstpot_5210
    );
  Modulation_read_pos_6 : X_SFF
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_6_rstpot_5219,
      O => Modulation_read_pos(6),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_6_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => X"F5A0DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_6_1_0,
      ADR2 => N79,
      ADR3 => Modulation_read_pos(6),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_6_rstpot_5219
    );
  Modulation_read_pos_5 : X_SFF
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_5_rstpot_5227,
      O => Modulation_read_pos(5),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_5_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_5_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(5),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_5_rstpot_5227
    );
  Modulation_read_pos_4 : X_SFF
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_4_rstpot_5235,
      O => Modulation_read_pos(4),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_4_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y27",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_4_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(4),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_4_rstpot_5235
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_Q : X_LUT6
    generic map(
      LOC => "SLICE_X17Y28",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(9),
      ADR5 => Modulation_read_pos(9),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_Q_6732
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_Q : X_LUT6
    generic map(
      LOC => "SLICE_X17Y28",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(10),
      ADR5 => Modulation_read_pos(10),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_Q_6649
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_Q : X_LUT6
    generic map(
      LOC => "SLICE_X17Y29",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(6),
      ADR5 => Modulation_read_pos(6),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_6_Q_6731
    );
  SPI_mod_second_byte : X_SFF
    generic map(
      LOC => "SLICE_X17Y30",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_mod_second_byte_glue_set,
      O => SPI_mod_second_byte_6429,
      SRST => Inst_ResetDelay_OUT_RESET_5868,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_n04021 : X_LUT6
    generic map(
      LOC => "SLICE_X17Y30",
      INIT => X"00FF00FF00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_second_byte_6429,
      ADR4 => '1',
      ADR5 => SPI_mod_first_byte_6405,
      O => SPI_mod_second_byte_glue_set
    );
  SPI_mod_lookup_buffer_39_SPI_mod_lookup_buffer_39_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(55),
      O => SPI_mod_lookup_buffer_55_0
    );
  SPI_mod_lookup_buffer_39_SPI_mod_lookup_buffer_39_CMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(54),
      O => SPI_mod_lookup_buffer_54_0
    );
  SPI_mod_lookup_buffer_39_SPI_mod_lookup_buffer_39_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(53),
      O => SPI_mod_lookup_buffer_53_0
    );
  SPI_mod_lookup_buffer_39_SPI_mod_lookup_buffer_39_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => SPI_mod_lookup_buffer(52),
      O => SPI_mod_lookup_buffer_52_0
    );
  SPI_mod_lookup_buffer_39 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_23_0,
      O => SPI_mod_lookup_buffer(39),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_39_rt : X_LUT5
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(39),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_39_rt_5267
    );
  SPI_mod_lookup_buffer_55 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_39_rt_5267,
      O => SPI_mod_lookup_buffer(55),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_38 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_22_0,
      O => SPI_mod_lookup_buffer(38),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_38_rt : X_LUT5
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(38),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_38_rt_5274
    );
  SPI_mod_lookup_buffer_54 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_38_rt_5274,
      O => SPI_mod_lookup_buffer(54),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_37 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_21_0,
      O => SPI_mod_lookup_buffer(37),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_37_rt : X_LUT5
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(37),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_37_rt_5257
    );
  SPI_mod_lookup_buffer_53 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_37_rt_5257,
      O => SPI_mod_lookup_buffer(53),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_36 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_20_0,
      O => SPI_mod_lookup_buffer(36),
      RST => GND,
      SET => GND
    );
  SPI_mod_lookup_buffer_36_rt : X_LUT5
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => X"FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_mod_lookup_buffer(36),
      ADR4 => '1',
      O => SPI_mod_lookup_buffer_36_rt_5262
    );
  SPI_mod_lookup_buffer_52 : X_FF
    generic map(
      LOC => "SLICE_X17Y36",
      INIT => '0'
    )
    port map (
      CE => SPI_n0578_inv1_6656,
      CLK => clk_pll,
      I => SPI_mod_lookup_buffer_36_rt_5262,
      O => SPI_mod_lookup_buffer(52),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_71 : X_FF
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_71_dpot_5290,
      O => SPI_MOD_LOOKUP_DATA(71),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_71_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_55_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(71),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_71_dpot_5290
    );
  SPI_MOD_LOOKUP_DATA_70 : X_FF
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_70_dpot_5299,
      O => SPI_MOD_LOOKUP_DATA(70),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_70_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_54_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(70),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_70_dpot_5299
    );
  SPI_MOD_LOOKUP_DATA_69 : X_FF
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_69_dpot_5306,
      O => SPI_MOD_LOOKUP_DATA(69),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_69_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_53_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(69),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_69_dpot_5306
    );
  SPI_MOD_LOOKUP_DATA_68 : X_FF
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => '0'
    )
    port map (
      CE => SPI_n0419_13_112,
      CLK => clk_pll,
      I => SPI_MOD_LOOKUP_DATA_68_dpot_5283,
      O => SPI_MOD_LOOKUP_DATA(68),
      RST => GND,
      SET => GND
    );
  SPI_MOD_LOOKUP_DATA_68_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X17Y37",
      INIT => X"FF00CFC0FF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => SPI_mod_lookup_buffer_52_0,
      ADR2 => SPI_state_FSM_FFd2_6400,
      ADR3 => SPI_MOD_LOOKUP_DATA(68),
      ADR4 => SPI_state_FSM_FFd1_5903,
      ADR5 => SPI_GND_17_o_word_cnt_4_equal_23_o,
      O => SPI_MOD_LOOKUP_DATA_68_dpot_5283
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o115 : X_LUT6
    generic map(
      LOC => "SLICE_X18Y25",
      INIT => X"0000FFFF00000000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(2),
      ADR5 => Modulation_read_pos(2),
      O => Modulation_read_pos_10_write_pos_10_equal_9_o114_6702
    );
  SPI_SPI_COMPLETE_1 : X_SFF
    generic map(
      LOC => "SLICE_X18Y25",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_SPI_data_synced_0_inv,
      O => SPI_SPI_COMPLETE_1_6693,
      SRST => SPI_SPI_data_valid_1_inv,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  SPI_SPI_data_synced_0_inv1_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X18Y25",
      INIT => X"00000000FFFFFFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => '1',
      ADR5 => SPI_SPI_data_synced_0_6630,
      O => SPI_SPI_data_synced_0_inv
    );
  Source_latched_regs_38 : X_FF
    generic map(
      LOC => "SLICE_X18Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_38_dpot_5318,
      O => Source_latched_regs(38),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_38_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X18Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(37),
      ADR2 => mod_lookup_read_data(38),
      ADR3 => Source_latched_regs(38),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_38_dpot_5318
    );
  Source_n0148_inv11_rstpot_2 : X_LUT6
    generic map(
      LOC => "SLICE_X18Y36",
      INIT => X"F0F0F0F00000CC00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_sclk_5839,
      ADR2 => source_reload_5964,
      ADR3 => Source_clk_cnt(2),
      ADR4 => Source_bit_cnt(5),
      ADR5 => Source_done_int_5965,
      O => Source_n0148_inv11_rstpot2
    );
  Source_latched_regs_37 : X_FF
    generic map(
      LOC => "SLICE_X18Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_37_dpot_5333,
      O => Source_latched_regs(37),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_37_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X18Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(36),
      ADR2 => mod_lookup_read_data(37),
      ADR3 => Source_latched_regs(37),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_37_dpot_5333
    );
  Source_latched_regs_36 : X_FF
    generic map(
      LOC => "SLICE_X18Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_36_dpot_5340,
      O => Source_latched_regs(36),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_36_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X18Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(35),
      ADR2 => mod_lookup_read_data(36),
      ADR3 => Source_latched_regs(36),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot1,
      O => Source_latched_regs_36_dpot_5340
    );
  Source_latched_regs_71 : X_FF
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_71_dpot_5367,
      O => Source_latched_regs(71),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_71_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(70),
      ADR2 => mod_lookup_read_data(71),
      ADR3 => Source_latched_regs(71),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_71_dpot_5367
    );
  Source_latched_regs_70 : X_FF
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_70_dpot_5346,
      O => Source_latched_regs(70),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_70_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(69),
      ADR2 => mod_lookup_read_data(70),
      ADR3 => Source_latched_regs(70),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_70_dpot_5346
    );
  Source_latched_regs_69 : X_FF
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_69_dpot_5353,
      O => Source_latched_regs(69),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_69_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(68),
      ADR2 => mod_lookup_read_data(69),
      ADR3 => Source_latched_regs(69),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_69_dpot_5353
    );
  Source_latched_regs_68 : X_FF
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_68_dpot_5360,
      O => Source_latched_regs(68),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_68_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X18Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(67),
      ADR2 => mod_lookup_read_data(68),
      ADR3 => Source_latched_regs(68),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_68_dpot_5360
    );
  SPI_SPI_COMPLETE : X_SFF
    generic map(
      LOC => "SLICE_X19Y25",
      INIT => '0'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => SPI_SPI_data_synced_0_inv,
      O => SPI_SPI_COMPLETE_6402,
      SRST => SPI_SPI_data_valid_1_inv,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_Q : X_LUT6
    generic map(
      LOC => "SLICE_X19Y25",
      INIT => X"FFFF00000000FFFF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => '1',
      ADR4 => Modulation_write_pos(3),
      ADR5 => Modulation_read_pos(3),
      O => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_3_Q_6730
    );
  Modulation_read_pos_3 : X_SFF
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_3_rstpot_5406,
      O => Modulation_read_pos(3),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_3_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_3_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(3),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_3_rstpot_5406
    );
  Modulation_read_pos_2 : X_SFF
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_2_rstpot_5415,
      O => Modulation_read_pos(2),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_2_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_2_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(2),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_2_rstpot_5415
    );
  Modulation_read_pos_1 : X_SFF
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_1_rstpot_5389,
      O => Modulation_read_pos(1),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_1_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_1_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(1),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_1_rstpot_5389
    );
  Modulation_read_pos_0 : X_SFF
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_0_rstpot_5397,
      O => Modulation_read_pos(0),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_0_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y26",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_0_1_0,
      ADR2 => N85,
      ADR3 => Modulation_read_pos(0),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_0_rstpot_5397
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o118 : X_LUT6
    generic map(
      LOC => "SLICE_X19Y27",
      INIT => X"F00F00000000F00F"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Modulation_write_pos(10),
      ADR3 => Modulation_read_pos(10),
      ADR4 => Modulation_write_pos(9),
      ADR5 => Modulation_read_pos(9),
      O => Modulation_read_pos_10_write_pos_10_equal_9_o117_6703
    );
  Modulation_read_pos_6_rstpot_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X19Y27",
      INIT => X"CFCFC0C0CFCFC840"
    )
    port map (
      ADR0 => Modulation_read_pos(7),
      ADR1 => Modulation_read_pos(6),
      ADR2 => Modulation_read_pos_10_write_pos_10_equal_9_o117_6703,
      ADR3 => N24,
      ADR4 => Modulation_Result_6_1_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o61,
      O => N79
    );
  Modulation_read_pos_0_rstpot_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X19Y27",
      INIT => X"FF00FF00FFC03F00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Modulation_read_pos(6),
      ADR2 => Modulation_read_pos(7),
      ADR3 => Modulation_read_pos_10_write_pos_10_equal_9_o117_6703,
      ADR4 => N24,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o61,
      O => N85
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o1110_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X19Y27",
      INIT => X"0C0330C0C0300C03"
    )
    port map (
      ADR0 => '1',
      ADR1 => Modulation_write_pos(10),
      ADR2 => Modulation_read_pos(8),
      ADR3 => Modulation_read_pos(10),
      ADR4 => Modulation_write_pos(9),
      ADR5 => Modulation_read_pos(9),
      O => N24
    );
  Modulation_read_pos_10_rstpot_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X19Y28",
      INIT => X"F000F000F8887000"
    )
    port map (
      ADR0 => Modulation_read_pos(6),
      ADR1 => Modulation_read_pos(7),
      ADR2 => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_9_Q_6732,
      ADR3 => Modulation_Msub_GND_22_o_GND_22_o_sub_1_OUT_10_0_lut_10_Q_6649,
      ADR4 => N24,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o61,
      O => N61
    );
  Modulation_read_pos_10 : X_SFF
    generic map(
      LOC => "SLICE_X19Y28",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_10_rstpot_5449,
      O => Modulation_read_pos(10),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_10_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y28",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_10_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(10),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_10_rstpot_5449
    );
  Modulation_read_pos_9 : X_SFF
    generic map(
      LOC => "SLICE_X19Y28",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_9_rstpot_5457,
      O => Modulation_read_pos(9),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_9_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y28",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_9_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(9),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_9_rstpot_5457
    );
  Modulation_read_pos_8 : X_SFF
    generic map(
      LOC => "SLICE_X19Y28",
      INIT => '1'
    )
    port map (
      CE => VCC,
      CLK => clk_pll,
      I => Modulation_read_pos_8_rstpot_5465,
      O => Modulation_read_pos(8),
      SSET => mod_reset,
      SET => GND,
      RST => GND,
      SRST => GND
    );
  Modulation_read_pos_8_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y28",
      INIT => X"FD08DD88DD88DD88"
    )
    port map (
      ADR0 => Modulation_clk_sample_cnt(26),
      ADR1 => Modulation_Result_8_1_0,
      ADR2 => N61,
      ADR3 => Modulation_read_pos(8),
      ADR4 => Modulation_read_pos_10_write_pos_10_equal_9_o116_0,
      ADR5 => Modulation_read_pos_10_write_pos_10_equal_9_o115_6492,
      O => Modulation_read_pos_8_rstpot_5465
    );
  Modulation_read_pos_10_write_pos_10_equal_9_o114 : X_LUT6
    generic map(
      LOC => "SLICE_X19Y29",
      INIT => X"0120804010020804"
    )
    port map (
      ADR0 => Modulation_write_pos(4),
      ADR1 => Modulation_write_pos(3),
      ADR2 => Modulation_write_pos(5),
      ADR3 => Modulation_read_pos(4),
      ADR4 => Modulation_read_pos(3),
      ADR5 => Modulation_read_pos(5),
      O => Modulation_read_pos_10_write_pos_10_equal_9_o113_6701
    );
  LEDS_5_OBUF_LEDS_5_OBUF_AMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => LEDS_4_OBUF_5482,
      O => LEDS_4_OBUF_0
    );
  LEDS_5_1 : X_LUT6
    generic map(
      LOC => "SLICE_X19Y32",
      INIT => X"FFFF00FFFFFF00FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_PORTSELECT_5829,
      ADR4 => SPI_AMP_SHDN_5830,
      ADR5 => '1',
      O => LEDS_5_OBUF_5836
    );
  LEDS_4_1 : X_LUT5
    generic map(
      LOC => "SLICE_X19Y32",
      INIT => X"FFFFFF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => SPI_SOURCE_PORTSELECT_5829,
      ADR4 => SPI_AMP_SHDN_5830,
      O => LEDS_4_OBUF_5482
    );
  Source_latched_regs_61 : X_FF
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_61_dpot_5507,
      O => Source_latched_regs(61),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_61_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(60),
      ADR2 => mod_lookup_read_data(61),
      ADR3 => Source_latched_regs(61),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_61_dpot_5507
    );
  Source_latched_regs_60 : X_FF
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_60_dpot_5486,
      O => Source_latched_regs(60),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_60_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(59),
      ADR2 => mod_lookup_read_data(60),
      ADR3 => Source_latched_regs(60),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_60_dpot_5486
    );
  Source_latched_regs_59 : X_FF
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_59_dpot_5493,
      O => Source_latched_regs(59),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_59_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(58),
      ADR2 => mod_lookup_read_data(59),
      ADR3 => Source_latched_regs(59),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_59_dpot_5493
    );
  Source_latched_regs_58 : X_FF
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_58_dpot_5500,
      O => Source_latched_regs(58),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_58_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(57),
      ADR2 => mod_lookup_read_data(58),
      ADR3 => Source_latched_regs(58),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_58_dpot_5500
    );
  Source_latched_regs_67 : X_FF
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_67_dpot_5537,
      O => Source_latched_regs(67),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_67_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(66),
      ADR2 => mod_lookup_read_data(67),
      ADR3 => Source_latched_regs(67),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_67_dpot_5537
    );
  Source_latched_regs_66 : X_FF
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_66_dpot_5516,
      O => Source_latched_regs(66),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_66_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(65),
      ADR2 => mod_lookup_read_data(66),
      ADR3 => Source_latched_regs(66),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_66_dpot_5516
    );
  Source_latched_regs_65 : X_FF
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_65_dpot_5523,
      O => Source_latched_regs(65),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_65_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(64),
      ADR2 => mod_lookup_read_data(65),
      ADR3 => Source_latched_regs(65),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_65_dpot_5523
    );
  Source_latched_regs_64 : X_FF
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot2,
      CLK => clk_pll,
      I => Source_latched_regs_64_dpot_5530,
      O => Source_latched_regs(64),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_64_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X19Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(63),
      ADR2 => mod_lookup_read_data(64),
      ADR3 => Source_latched_regs(64),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot3,
      O => Source_latched_regs_64_dpot_5530
    );
  inv_clk_locked_inv_clk_locked_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Inst_ResetDelay_clk_cnt_1_pack_1,
      O => Inst_ResetDelay_clk_cnt(1)
    );
  inv_clk_locked1 : X_LUT6
    generic map(
      LOC => "SLICE_X20Y15",
      INIT => X"000000FF000000FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => LEDS_1_OBUF_5931,
      ADR4 => RESET_IBUF_0,
      ADR5 => '1',
      O => inv_clk_locked
    );
  Inst_ResetDelay_Mcount_clk_cnt_xor_1_11 : X_LUT5
    generic map(
      LOC => "SLICE_X20Y15",
      INIT => X"3C3C3C3C"
    )
    port map (
      ADR0 => '1',
      ADR1 => Inst_ResetDelay_clk_cnt(1),
      ADR2 => Inst_ResetDelay_clk_cnt(0),
      ADR3 => '1',
      ADR4 => '1',
      O => Result(1)
    );
  Inst_ResetDelay_clk_cnt_1 : X_SFF
    generic map(
      LOC => "SLICE_X20Y15",
      INIT => '0'
    )
    port map (
      CE => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o,
      CLK => clk_pll,
      I => Result(1),
      O => Inst_ResetDelay_clk_cnt_1_pack_1,
      SRST => inv_clk_locked,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Source_latched_regs_42 : X_FF
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_42_dpot_5578,
      O => Source_latched_regs(42),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_42_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(41),
      ADR2 => mod_lookup_read_data(42),
      ADR3 => Source_latched_regs(42),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_42_dpot_5578
    );
  Source_latched_regs_41 : X_FF
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_41_dpot_5557,
      O => Source_latched_regs(41),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_41_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(40),
      ADR2 => mod_lookup_read_data(41),
      ADR3 => Source_latched_regs(41),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_41_dpot_5557
    );
  Source_latched_regs_40 : X_FF
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_40_dpot_5564,
      O => Source_latched_regs(40),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_40_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(39),
      ADR2 => mod_lookup_read_data(40),
      ADR3 => Source_latched_regs(40),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_40_dpot_5564
    );
  Source_latched_regs_39 : X_FF
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_39_dpot_5571,
      O => Source_latched_regs(39),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_39_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y36",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(38),
      ADR2 => mod_lookup_read_data(39),
      ADR3 => Source_latched_regs(39),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_39_dpot_5571
    );
  Source_latched_regs_54 : X_FF
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_54_dpot_5608,
      O => Source_latched_regs(54),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_54_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(53),
      ADR2 => mod_lookup_read_data(54),
      ADR3 => Source_latched_regs(54),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_54_dpot_5608
    );
  Source_latched_regs_53 : X_FF
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_53_dpot_5587,
      O => Source_latched_regs(53),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_53_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(52),
      ADR2 => mod_lookup_read_data(53),
      ADR3 => Source_latched_regs(53),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_53_dpot_5587
    );
  Source_latched_regs_52 : X_FF
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_52_dpot_5594,
      O => Source_latched_regs(52),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_52_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(51),
      ADR2 => mod_lookup_read_data(52),
      ADR3 => Source_latched_regs(52),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_52_dpot_5594
    );
  Source_latched_regs_51 : X_FF
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_51_dpot_5601,
      O => Source_latched_regs(51),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_51_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(50),
      ADR2 => mod_lookup_read_data(51),
      ADR3 => Source_latched_regs(51),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_51_dpot_5601
    );
  Source_latched_regs_50 : X_FF
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_50_dpot_5638,
      O => Source_latched_regs(50),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_50_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(49),
      ADR2 => mod_lookup_read_data(50),
      ADR3 => Source_latched_regs(50),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_50_dpot_5638
    );
  Source_latched_regs_49 : X_FF
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_49_dpot_5617,
      O => Source_latched_regs(49),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_49_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(48),
      ADR2 => mod_lookup_read_data(49),
      ADR3 => Source_latched_regs(49),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_49_dpot_5617
    );
  Source_latched_regs_48 : X_FF
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_48_dpot_5624,
      O => Source_latched_regs(48),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_48_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(47),
      ADR2 => mod_lookup_read_data(48),
      ADR3 => Source_latched_regs(48),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_48_dpot_5624
    );
  Source_latched_regs_47 : X_FF
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_47_dpot_5631,
      O => Source_latched_regs(47),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_47_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X20Y38",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(46),
      ADR2 => mod_lookup_read_data(47),
      ADR3 => Source_latched_regs(47),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_47_dpot_5631
    );
  Source_latched_regs_46 : X_FF
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_46_dpot_5668,
      O => Source_latched_regs(46),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_46_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(45),
      ADR2 => mod_lookup_read_data(46),
      ADR3 => Source_latched_regs(46),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_46_dpot_5668
    );
  Source_latched_regs_45 : X_FF
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_45_dpot_5647,
      O => Source_latched_regs(45),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_45_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(44),
      ADR2 => mod_lookup_read_data(45),
      ADR3 => Source_latched_regs(45),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_45_dpot_5647
    );
  Source_latched_regs_44 : X_FF
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_44_dpot_5654,
      O => Source_latched_regs(44),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_44_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(43),
      ADR2 => mod_lookup_read_data(44),
      ADR3 => Source_latched_regs(44),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_44_dpot_5654
    );
  Source_latched_regs_43 : X_FF
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => '0'
    )
    port map (
      CE => Source_n0148_inv11_cepot1,
      CLK => clk_pll,
      I => Source_latched_regs_43_dpot_5661,
      O => Source_latched_regs(43),
      RST => GND,
      SET => GND
    );
  Source_latched_regs_43_dpot : X_LUT6
    generic map(
      LOC => "SLICE_X21Y37",
      INIT => X"F0F0CCCCFF00FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => Source_latched_regs(42),
      ADR2 => mod_lookup_read_data(43),
      ADR3 => Source_latched_regs(43),
      ADR4 => Source_done_int_5965,
      ADR5 => Source_n0148_inv11_rstpot2,
      O => Source_latched_regs_43_dpot_5661
    );
  Inst_ResetDelay_clk_cnt_6_Inst_ResetDelay_clk_cnt_6_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => N4_pack_16,
      O => N4
    );
  Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o1_SW0 : X_LUT6
    generic map(
      LOC => "SLICE_X22Y15",
      INIT => X"FFFFFF00FFFFFF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Inst_ResetDelay_clk_cnt(4),
      ADR4 => Inst_ResetDelay_clk_cnt(3),
      ADR5 => '1',
      O => N0
    );
  Result_6_SW0 : X_LUT5
    generic map(
      LOC => "SLICE_X22Y15",
      INIT => X"F0F00000"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Inst_ResetDelay_clk_cnt(5),
      ADR3 => '1',
      ADR4 => Inst_ResetDelay_clk_cnt(3),
      O => N4_pack_16
    );
  Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o1 : X_LUT6
    generic map(
      LOC => "SLICE_X22Y15",
      INIT => X"77777777777F7F7F"
    )
    port map (
      ADR0 => Inst_ResetDelay_clk_cnt(6),
      ADR1 => Inst_ResetDelay_clk_cnt(5),
      ADR2 => Inst_ResetDelay_clk_cnt(2),
      ADR3 => Inst_ResetDelay_clk_cnt(1),
      ADR4 => Inst_ResetDelay_clk_cnt(0),
      ADR5 => N0,
      O => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o
    );
  Inst_ResetDelay_clk_cnt_6 : X_SFF
    generic map(
      LOC => "SLICE_X22Y15",
      INIT => '0'
    )
    port map (
      CE => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o,
      CLK => clk_pll,
      I => Result(6),
      O => Inst_ResetDelay_clk_cnt(6),
      SRST => inv_clk_locked,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Result_6_Q : X_LUT6
    generic map(
      LOC => "SLICE_X22Y15",
      INIT => X"7F80FF00FF00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_clk_cnt(2),
      ADR1 => Inst_ResetDelay_clk_cnt(1),
      ADR2 => Inst_ResetDelay_clk_cnt(0),
      ADR3 => Inst_ResetDelay_clk_cnt(6),
      ADR4 => Inst_ResetDelay_clk_cnt(4),
      ADR5 => N4,
      O => Result(6)
    );
  Inst_ResetDelay_clk_cnt_5 : X_SFF
    generic map(
      LOC => "SLICE_X22Y15",
      INIT => '0'
    )
    port map (
      CE => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o,
      CLK => clk_pll,
      I => Result(5),
      O => Inst_ResetDelay_clk_cnt(5),
      SRST => inv_clk_locked,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Result_5_1 : X_LUT6
    generic map(
      LOC => "SLICE_X22Y15",
      INIT => X"7F80FF00FF00FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_clk_cnt(3),
      ADR1 => Inst_ResetDelay_clk_cnt(4),
      ADR2 => Inst_ResetDelay_clk_cnt(0),
      ADR3 => Inst_ResetDelay_clk_cnt(5),
      ADR4 => Inst_ResetDelay_clk_cnt(1),
      ADR5 => Inst_ResetDelay_clk_cnt(2),
      O => Result(5)
    );
  Inst_ResetDelay_clk_cnt_4_Inst_ResetDelay_clk_cnt_4_DMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => N91_pack_9,
      O => N91
    );
  Inst_ResetDelay_clk_cnt_4_Inst_ResetDelay_clk_cnt_4_BMUX_Delay : X_BUF
    generic map(
      PATHPULSE => 115 ps
    )
    port map (
      I => Inst_ResetDelay_clk_cnt_3_pack_7,
      O => Inst_ResetDelay_clk_cnt(3)
    );
  Inst_ResetDelay_clk_cnt_4 : X_SFF
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => '0'
    )
    port map (
      CE => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o,
      CLK => clk_pll,
      I => Result(4),
      O => Inst_ResetDelay_clk_cnt(4),
      SRST => inv_clk_locked,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Result_4_1 : X_LUT6
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => X"7F80FF007F80FF00"
    )
    port map (
      ADR0 => Inst_ResetDelay_clk_cnt(3),
      ADR1 => Inst_ResetDelay_clk_cnt(0),
      ADR2 => Inst_ResetDelay_clk_cnt(1),
      ADR3 => Inst_ResetDelay_clk_cnt(4),
      ADR4 => Inst_ResetDelay_clk_cnt(2),
      ADR5 => '1',
      O => Result(4)
    );
  Inst_ResetDelay_OUT_RESET_rstpot_SW1 : X_LUT5
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => X"FFFFFFEA"
    )
    port map (
      ADR0 => Inst_ResetDelay_clk_cnt(3),
      ADR1 => Inst_ResetDelay_clk_cnt(0),
      ADR2 => Inst_ResetDelay_clk_cnt(1),
      ADR3 => Inst_ResetDelay_clk_cnt(4),
      ADR4 => Inst_ResetDelay_clk_cnt(2),
      O => N91_pack_9
    );
  Inst_ResetDelay_OUT_RESET_rstpot : X_LUT6
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => X"2AAA2AAA2AAAFFFF"
    )
    port map (
      ADR0 => Inst_ResetDelay_OUT_RESET_5868,
      ADR1 => N91,
      ADR2 => Inst_ResetDelay_clk_cnt(5),
      ADR3 => Inst_ResetDelay_clk_cnt(6),
      ADR4 => LEDS_1_OBUF_5931,
      ADR5 => RESET_IBUF_0,
      O => Inst_ResetDelay_OUT_RESET_rstpot_5930
    );
  Inst_ResetDelay_clk_cnt_2 : X_SFF
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => '0'
    )
    port map (
      CE => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o,
      CLK => clk_pll,
      I => Result(2),
      O => Inst_ResetDelay_clk_cnt(2),
      SRST => inv_clk_locked,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Inst_ResetDelay_Mcount_clk_cnt_xor_2_11 : X_LUT6
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => X"0FF0FF000FF0FF00"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => Inst_ResetDelay_clk_cnt(0),
      ADR3 => Inst_ResetDelay_clk_cnt(2),
      ADR4 => Inst_ResetDelay_clk_cnt(1),
      ADR5 => '1',
      O => Result(2)
    );
  Result_3_1 : X_LUT5
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => X"3CCCCCCC"
    )
    port map (
      ADR0 => '1',
      ADR1 => Inst_ResetDelay_clk_cnt(3),
      ADR2 => Inst_ResetDelay_clk_cnt(0),
      ADR3 => Inst_ResetDelay_clk_cnt(2),
      ADR4 => Inst_ResetDelay_clk_cnt(1),
      O => Result(3)
    );
  Inst_ResetDelay_clk_cnt_3 : X_SFF
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => '0'
    )
    port map (
      CE => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o,
      CLK => clk_pll,
      I => Result(3),
      O => Inst_ResetDelay_clk_cnt_3_pack_7,
      SRST => inv_clk_locked,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Inst_ResetDelay_clk_cnt_0 : X_SFF
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => '0'
    )
    port map (
      CE => Inst_ResetDelay_clk_cnt_6_PWR_9_o_LessThan_1_o,
      CLK => clk_pll,
      I => Result(0),
      O => Inst_ResetDelay_clk_cnt(0),
      SRST => inv_clk_locked,
      SET => GND,
      RST => GND,
      SSET => GND
    );
  Inst_ResetDelay_Mcount_clk_cnt_xor_0_11_INV_0 : X_LUT6
    generic map(
      LOC => "SLICE_X23Y15",
      INIT => X"00FF00FF00FF00FF"
    )
    port map (
      ADR0 => '1',
      ADR1 => '1',
      ADR2 => '1',
      ADR3 => Inst_ResetDelay_clk_cnt(0),
      ADR4 => '1',
      ADR5 => '1',
      O => Result(0)
    );
  NlwBlock_top_GND : X_ZERO
    port map (
      O => GND
    );
  NlwBlock_top_VCC : X_ONE
    port map (
      O => VCC
    );
  NlwBlockROC : X_ROC
    generic map (ROC_WIDTH => 100 ns)
    port map (O => GSR);
  NlwBlockTOC : X_TOC
    port map (O => GTS);

end Structure;

