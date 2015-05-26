host=['lyoilcrpi03','lyoilcrpi04','lyoilcrpi05','lyoilcrpi06','lyoilcrpi07','lyoilcrpi08','lyoilcrpi09','lyoilcrpi10','lyoilcrpi11','lyoilcrpi12','lyoilcrpi14','lyoilcrpi15','lyoilcrpi19']
#'lyoilcrpi15',
ccc='lyoilcrpi17'
zuphost='lyoilcrpi17'
zupdevice='/dev/ttyUSB0'
zupport=1
db='lyosdhcal11'
writer='lyosdhcal11'
register=0x89580000
#register=0x815a1b00
# state 180=state 179 + kill 11 noisyest channel
# state 179=state 175 with gain=128 in multi gap and few channels with gain divided by 2 (those with ;ore than 10 hits per trigger (in arnaud analysis where coherent noise are not counted))
# state 178=state 175 with gain=64 in multi gap
# state 176=state 175 with gain=128 in multi gap
#state="Dome_42chambres_Reference_v4_175"
#state="Dome_42chambres_Reference_v4_180"
# State 145 with gain adjustment of arnaud  with max 254 and few noisy bads gain divided by two
# State 175=state 171 with 3 chanel masked in chamber 50 (multigap)
#state="Dome_42chambres_Reference_v4_175"
# State 170 with thresholds of the difs 80,105,182 changed to be like others
#state="Dome_42chambres_Reference_v4_171"
#state="Dome_42chambres_Reference_v4_170"
# State 145 with gain adjustment of arnaud  with max 254
#state="Dome_42chambres_Reference_v4_169"
# State 145 with gain adjustment of arnaud  with max 147
state="Dome_42chambres_Reference_v4_168"
# State 145 with 128 on all PAGAIN
#state="Dome_42chambres_Reference_v4_167"
# State 180 masque LM
# state="Dome_42chambres_Reference_v4_162"
# 115 + petit seup
# State 170  arnaud mask +scaling
#state="Dome_42chambres_Reference_v4_161"
# State 170  arnaud mask
#state="Dome_42chambres_Reference_v4_160"
# State 170  no mask
#state="Dome_42chambres_Reference_v4_145"
# State 170 masque LM
#state="Dome_42chambres_Reference_v4_158"
#state="Dome_42chambres_Reference_v4_163"

# State 170  arnaud mask +scaling (<gain>=1.0)
#state="Dome_42chambres_Reference_v4_164"
# State 170  arnaud mask +scaling (lowest gain=0.5)
#state="Dome_42chambres_Reference_v4_166"

# State 181 derivate from state 180 with first threshold at 190
#state="Dome_42chambres_Reference_v4_181"

# State 182 derivate from state 180 with first threshold at 220
#state="Dome_42chambres_Reference_v4_182"


# Novermber 2012 state="Dome_42chambres_Reference_v4_115"
# November 2012 + masks state="Dome_42chambres_Reference_v4_144"
directory="/data/NAS/Avril2015"
