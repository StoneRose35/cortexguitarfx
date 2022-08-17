
% calculation of best vco frequency and system frequency to satisfy the need of UART Communination
% and neopixel clocking
% atoms having the solution predicate are listed as follows
% POSTDIV1
% POSTDIV2
% NP_CLKDIV
% FEEDBK


postdiv1(1..7).
postdiv2(1..7).

ws2812_fact(1..166).

fxosc(12000).
feedbk(16..320).

fvco(Xo*Fb,Fb) :- fxosc(Xo), feedbk(Fb).
fsys(Xo*Fb/P1/P2,P1,P2,Fb) :- fxosc(Xo), feedbk(Fb), postdiv1(P1), postdiv2(P2).
fws2812(Fs/Wf/10,Wf,P1,P2,Fb) :- fsys(Fs,P1,P2,Fb), ws2812_fact(Wf).

1 { sol(P1,P2,Wf,Fd): postdiv1(P1), postdiv2(P2), ws2812_fact(Wf), feedbk(Fd), P1 >= P2 } 1.

% v_vco must be within 400MHz and 1600MHz
:- sol(P1,P2,Wf,Fd), fvco(Fv,Fd), Fv < 400000.
:- sol(P1,P2,Wf,Fd), fvco(Fv,Fd), Fv > 1600000.

% fsys must not be greated than 133MHz
:- sol(P1,P2,Wf,Fd), fsys(Fsys,P1,P2,Fd), Fsys > 120000.

% factors: deviation from ideal ws2812 frequency
ws2812err(|Fws-800|,Wf,P1,P2,Fb) :-sol(P1,P2,Wf,Fb), fws2812(Fws,Wf,P1,P2,Fb).

% vco frequency: should be as high/low as possible



#minimize { Werr@3 ,Wf,P1,P2,Fb: ws2812err(Werr,Wf,P1,P2,Fb), sol(P1,P2,Wf,Fb) }.
#minimize {-Fvco@1, Fb: fvco(Fvco,Fb),sol(_,_,_,Fb)}.
#minimize {-Fsys@2, P1,P2,Fb: fsys(Fsys,P1,P2,Fb),sol(P1,P2,_,Fb)}.
#show sol/4.
