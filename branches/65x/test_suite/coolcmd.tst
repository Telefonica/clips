(unwatch all)
(clear)
(conserve-mem off)
(dribble-on "Actual//coolcmd.out")
(batch "coolcmd.bat")
(dribble-off)
(clear)
(open "Results//coolcmd.rsl" coolcmd "w")
(load "compline.clp")
(printout coolcmd "coolcmd.bat differences are as follows:" crlf)
(compare-files "Expected//coolcmd.out" "Actual//coolcmd.out" coolcmd)
(printout coolcmd "coolcmd.ins differences are as follows:" crlf)
(compare-files "Expected//coolcmd2.out" "Actual//coolcmd2.out" coolcmd)
(close coolcmd)
