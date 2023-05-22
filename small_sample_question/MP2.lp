\ Model master
\ LP format - for model browsing. Use MPS format to capture full model detail.
Minimize
  x(0) + x(1) + x(2) + x(3) + x(4) + x(5)
Subject To
 order(0): x(3) + x(4) + x(5) >= 1
 order(1): x(2) + x(4) >= 1
 order(2): x(0) + x(5) >= 1
 order(3): x(1) >= 1
Bounds
Binaries
 x(0) x(1) x(2) x(3) x(4) x(5)
End
