rm gerber/*
rm gerbv/*
rm macrofab_gerber/*
rm oshpark_gerber/*
rm zofzpcb/*

SET PROJ_NAME=VNA

cp %PROJ_NAME%.drl gerber/%PROJ_NAME%.txt

cp %PROJ_NAME%-Edge_Cuts.gm1 gerber/%PROJ_NAME%.gml
cp %PROJ_NAME%-F_Paste.gtp gerber/%PROJ_NAME%.gtp
cp %PROJ_NAME%-B_Paste.gbp gerber/%PROJ_NAME%.gbp
cp %PROJ_NAME%-F_Mask.gts gerber/%PROJ_NAME%.gts
cp %PROJ_NAME%-B_Mask.gbs gerber/%PROJ_NAME%.gbs
cp %PROJ_NAME%-B_SilkS.gbo gerber/%PROJ_NAME%.gbo
cp %PROJ_NAME%-F_SilkS.gto gerber/%PROJ_NAME%.gto

cp %PROJ_NAME%-1_top.gtl gerber/%PROJ_NAME%.gtl
cp %PROJ_NAME%-2_gnd.g2 gerber/%PROJ_NAME%.gl2
cp %PROJ_NAME%-3_pwr.g3 gerber/%PROJ_NAME%.gl3
cp %PROJ_NAME%-4_bot.gbl gerber/%PROJ_NAME%.gbl

cp gerber/%PROJ_NAME%.txt gerbv/%PROJ_NAME%.txt
cp gerber/%PROJ_NAME%.gml gerbv/%PROJ_NAME%.gml
cp gerber/%PROJ_NAME%.gtp gerbv/%PROJ_NAME%.gtp
cp gerber/%PROJ_NAME%.gbp gerbv/%PROJ_NAME%.gbp
cp gerber/%PROJ_NAME%.gts gerbv/%PROJ_NAME%.gts
cp gerber/%PROJ_NAME%.gbs gerbv/%PROJ_NAME%.gbs
cp gerber/%PROJ_NAME%.gbo gerbv/%PROJ_NAME%.gbo
cp gerber/%PROJ_NAME%.gto gerbv/%PROJ_NAME%.gto
cp gerber/%PROJ_NAME%.gtl gerbv/%PROJ_NAME%.gtl
cp gerber/%PROJ_NAME%.gbl gerbv/%PROJ_NAME%.gbl
cp gerber/%PROJ_NAME%.gl2 gerbv/%PROJ_NAME%.gl2
cp gerber/%PROJ_NAME%.gl3 gerbv/%PROJ_NAME%.gl3

cp %PROJ_NAME%-1_top.gtl macrofab_gerber/%PROJ_NAME%.GTL
cp %PROJ_NAME%-F_SilkS.gto macrofab_gerber/%PROJ_NAME%.GTO
cp %PROJ_NAME%-F_Mask.gts macrofab_gerber/%PROJ_NAME%.GTS
cp %PROJ_NAME%-F_Paste.gtp macrofab_gerber/%PROJ_NAME%.GTP

cp %PROJ_NAME%-2_gnd.g2 macrofab_gerber/%PROJ_NAME%.G2L
cp %PROJ_NAME%-3_pwr.g3 macrofab_gerber/%PROJ_NAME%.G3L

cp %PROJ_NAME%-4_bot.gbl macrofab_gerber/%PROJ_NAME%.GBL
cp %PROJ_NAME%-B_SilkS.gbo macrofab_gerber/%PROJ_NAME%.GBO
cp %PROJ_NAME%-B_Mask.gbs macrofab_gerber/%PROJ_NAME%.GBS
cp %PROJ_NAME%-B_Paste.gbp macrofab_gerber/%PROJ_NAME%.GBP

cp %PROJ_NAME%-Edge_Cuts.gm1 macrofab_gerber/%PROJ_NAME%.BOR
cp %PROJ_NAME%.drl macrofab_gerber/%PROJ_NAME%.XLN

cp %PROJ_NAME%-Edge_Cuts.gm1 oshpark_gerber/%PROJ_NAME%-Edge_Cuts.GKO
cp %PROJ_NAME%-F_Paste.gtp oshpark_gerber/%PROJ_NAME%-F_Paste.GTP
cp %PROJ_NAME%-B_Paste.gbp oshpark_gerber/%PROJ_NAME%-B_Paste.GBP
cp %PROJ_NAME%-F_Mask.gts oshpark_gerber/%PROJ_NAME%-F_Mask.GTS
cp %PROJ_NAME%-B_Mask.gbs oshpark_gerber/%PROJ_NAME%-B_Mask.GBS
cp %PROJ_NAME%-B_SilkS.gbo oshpark_gerber/%PROJ_NAME%-B_SilkS.GBO
cp %PROJ_NAME%-F_SilkS.gto oshpark_gerber/%PROJ_NAME%-F_SilkS.GTO

cp %PROJ_NAME%-1_top.gtl oshpark_gerber/%PROJ_NAME%-Top.GTL
cp %PROJ_NAME%-2_gnd.g2 oshpark_gerber/%PROJ_NAME%-In1_Cu.G2L
cp %PROJ_NAME%-3_pwr.g3 oshpark_gerber/%PROJ_NAME%-In2_Cu.G3L
cp %PROJ_NAME%-4_bot.gbl oshpark_gerber/%PROJ_NAME%-Bottom.GBL
cp %PROJ_NAME%.drl oshpark_gerber/%PROJ_NAME%.XLN

cp gerber/%PROJ_NAME%.txt zofzpcb/%PROJ_NAME%.txt
cp gerber/%PROJ_NAME%.gml zofzpcb/%PROJ_NAME%.gml
cp gerber/%PROJ_NAME%.gtp zofzpcb/%PROJ_NAME%.gtp
cp gerber/%PROJ_NAME%.gbp zofzpcb/%PROJ_NAME%.gbp
cp gerber/%PROJ_NAME%.gts zofzpcb/%PROJ_NAME%.gts
cp gerber/%PROJ_NAME%.gbs zofzpcb/%PROJ_NAME%.gbs
cp gerber/%PROJ_NAME%.gbo zofzpcb/%PROJ_NAME%.gbo
cp gerber/%PROJ_NAME%.gto zofzpcb/%PROJ_NAME%.gto
cp gerber/%PROJ_NAME%.gtl zofzpcb/%PROJ_NAME%.gtl
cp gerber/%PROJ_NAME%.gbl zofzpcb/%PROJ_NAME%.gbl
cp gerber/%PROJ_NAME%.gl2 zofzpcb/%PROJ_NAME%.g2
cp gerber/%PROJ_NAME%.gl3 zofzpcb/%PROJ_NAME%.g3
cp %PROJ_NAME%.d356 zofzpcb/%PROJ_NAME%.d356
