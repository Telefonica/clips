(make-instance a of A)
(send [a] print)
(make-instance [a] of A (x 65))
(send [a] print)
(unmake-instance [a])
(make-instance b1 of BOGUS)
(make-instance b1 of BOGUS (b 45))
(send [b1] put-a 1)
(send [b1] put-b 2)
(send [b1] print)
(initialize-instance [b1] (a xyz))
(send [b1] print)
(unmake-instance [b1])
(make-instance b of B (z 0))
(send [b] print)
(send [b] put-x 65)
(send [b] put-y abc)
(send [b] put-z "Hello world.")
(send [b] print)
(initialize-instance b)
(send [b] print)
(unmake-instance b)
(watch instances)
(reset)
(reset)
(unwatch instances)
(unmake-instance *)
(make-instance d of D)
(sym-cat (send [d] get-x) def)
(send [d] put-x "New value.")
(send [d] get-x)
(unmake-instance *)
(make-instance z of Z)
(instances)
(unmake-instance *)
(unwatch all)
(watch messages)
(make-instance sv of SAVE-TEST (stamp 1))
(make-instance sv of SAVE-TEST)
(save-instances "Temp//ins.tmp")
(load-instances "Temp//ins.tmp")
(instances)
(restore-instances "Temp//ins.tmp")
(instances)
(send [sv] print)
