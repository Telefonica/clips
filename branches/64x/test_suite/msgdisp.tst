(unwatch all)
(clear)
(load "msgdisp.clp")
(dribble-on "Actual//msgdisp.out")
(testit)
(dribble-off)
(clear)
(open "Results//msgdisp.rsl" msgdisp "w")
(load "compline.clp")
(printout msgdisp "msgdisp.clp differences are as follows:" crlf)
(compare-files "Expected//msgdisp.out" "Actual//msgdisp.out" msgdisp)
(close msgdisp)
