(unwatch all)
(clear)
(load "msghand.clp")
(dribble-on "Actual//msghand.out")
(batch "msghand.bat")
(dribble-off)
(clear)
(open "Results//msghand.rsl" msghand "w")
(load "compline.clp")
(printout msghand "msghand.clp differences are as follows:" crlf)
(compare-files "Expected//msghand.out" "Actual//msghand.out" msghand)
(close msghand)
