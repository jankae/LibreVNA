
--------------------------------------------------------------------------------
--
-- BLK MEM GEN v7_3 Core - Testbench Package 
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
-- Filename: bmg_tb_pkg.vhd
--
-- Description:
--   BMG Testbench Package files
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

PACKAGE BMG_TB_PKG IS

 FUNCTION DIVROUNDUP (
    DATA_VALUE : INTEGER;
    DIVISOR : INTEGER)
 RETURN INTEGER;
 ------------------------
 FUNCTION IF_THEN_ELSE (
    CONDITION : BOOLEAN;
    TRUE_CASE : STD_LOGIC_VECTOR;
    FALSE_CASE : STD_LOGIC_VECTOR)
  RETURN STD_LOGIC_VECTOR;
 ------------------------ 
 FUNCTION IF_THEN_ELSE (
   CONDITION : BOOLEAN;
   TRUE_CASE : STRING;
   FALSE_CASE :STRING)
 RETURN STRING;
 ------------------------ 
 FUNCTION IF_THEN_ELSE (
   CONDITION : BOOLEAN;
   TRUE_CASE : STD_LOGIC;
   FALSE_CASE :STD_LOGIC)
 RETURN STD_LOGIC;
 ------------------------ 
 FUNCTION IF_THEN_ELSE (
   CONDITION : BOOLEAN;
   TRUE_CASE : INTEGER;
   FALSE_CASE : INTEGER)
 RETURN INTEGER;
 ------------------------ 
 FUNCTION LOG2ROUNDUP (
      DATA_VALUE : INTEGER)
 RETURN INTEGER; 

END BMG_TB_PKG;

PACKAGE BODY BMG_TB_PKG IS

 FUNCTION DIVROUNDUP (
    DATA_VALUE : INTEGER;
    DIVISOR : INTEGER)
  RETURN INTEGER IS
    VARIABLE DIV                   : INTEGER;
  BEGIN
    DIV   := DATA_VALUE/DIVISOR;
    IF ( (DATA_VALUE MOD DIVISOR) /= 0) THEN
      DIV := DIV+1;
    END IF;
    RETURN DIV;
  END DIVROUNDUP;
  ---------------------------------
    FUNCTION IF_THEN_ELSE (
    CONDITION : BOOLEAN;
    TRUE_CASE : STD_LOGIC_VECTOR;
    FALSE_CASE : STD_LOGIC_VECTOR)
    RETURN STD_LOGIC_VECTOR IS
    BEGIN
       IF NOT CONDITION THEN
          RETURN FALSE_CASE;
       ELSE
          RETURN TRUE_CASE;
      END IF;
   END IF_THEN_ELSE;
  ---------------------------------
    FUNCTION IF_THEN_ELSE (
    CONDITION : BOOLEAN;
    TRUE_CASE : STD_LOGIC;
    FALSE_CASE : STD_LOGIC)
    RETURN STD_LOGIC IS
    BEGIN
       IF NOT CONDITION THEN
          RETURN FALSE_CASE;
       ELSE
          RETURN TRUE_CASE;
      END IF;
   END IF_THEN_ELSE;
  ---------------------------------
  FUNCTION IF_THEN_ELSE (
    CONDITION : BOOLEAN;
    TRUE_CASE : INTEGER;
    FALSE_CASE : INTEGER)
  RETURN INTEGER IS
    VARIABLE RETVAL : INTEGER := 0;
  BEGIN
    IF CONDITION=FALSE THEN
      RETVAL:=FALSE_CASE;
    ELSE
      RETVAL:=TRUE_CASE;
    END IF;
    RETURN RETVAL;
  END IF_THEN_ELSE;
  ---------------------------------
  FUNCTION IF_THEN_ELSE (
    CONDITION : BOOLEAN;
    TRUE_CASE : STRING;
    FALSE_CASE : STRING)
  RETURN STRING IS
    BEGIN
     IF NOT CONDITION THEN
        RETURN FALSE_CASE;
     ELSE
        RETURN TRUE_CASE;
     END IF;
 END IF_THEN_ELSE;
  ------------------------------- 
  FUNCTION LOG2ROUNDUP (
      DATA_VALUE : INTEGER)
    RETURN INTEGER IS
      VARIABLE WIDTH       : INTEGER := 0;
      VARIABLE CNT         : INTEGER := 1;
    BEGIN
      IF (DATA_VALUE <= 1) THEN
        WIDTH   := 1;
      ELSE
        WHILE (CNT < DATA_VALUE) LOOP
          WIDTH := WIDTH + 1;
          CNT   := CNT *2;
        END LOOP;
      END IF;
      RETURN WIDTH;
    END LOG2ROUNDUP;

END BMG_TB_PKG;
