*      ========  ENTRY  ========
   0:    LDC    6,1023(   0) * SP = 1023
   1:    LDA    6,  -1(   6) * push BP
   2:     ST    5,   0(   6) * ...
   3:    LDC    0,   8(   0) * push return address
   4:    LDA    6,  -1(   6) * ...
   5:     ST    0,   0(   6) * ...
   6:    LDA    5,   0(   6) * BP = SP
   7:    LDC    7, 285(   0) * call function
   8:    LDA    6,   0(   5) * SP = BP
   9:    LDA    6,   1(   6) * SP += 1
  10:     LD    5,   0(   6) * pop BP
  11:    LDA    6,   1(   6) * ...
  12:   HALT    0,   0,   0 
* 
* FunDef: input
  13:     IN    0,   0,   0  * input to AX
  14:     LD    7,   0(   5) * ret
* 
* FunDef: output
  15:     LD    0,   1(   5) * AX = data[BP + (1)]
  16:    OUT    0,   0,   0  * output AX
  17:     LD    7,   0(   5) * ret
* 
* FunDef: f
*   Compoundstmt:
  18:    LDA    6,   0(   6) * SP += 0
*     If:
*       =>cond
*       Op:
  19:    LDA    6,  -1(   6) * push BX
  20:     ST    1,   0(   6) * ...
  21:     LD    0,   1(   5) * var value: n
  22:    LDA    1,   0(   0) * BX = AX
  23:    LDC    0,   0(   0) * num 0
  24:    SUB    0,   1,   0  * test and jmp
  25:    JLE    0,   2(   7) * ...
  26:    LDC    0,   0(   0) * set false
  27:    LDC    7,  29(   0) * skip set true
  28:    LDC    0,   1(   0) * set true
  29:     LD    1,   0(   6) * pop BX
  30:    LDA    6,   1(   6) * ...
*       |
  31:    JEQ    0,   5(   7)
*       =>then
*       Compoundstmt:
  32:    LDA    6,   0(   6) * SP += 0
*         Return:
  33:    LDC    0,   0(   0) * num 0
  34:     LD    7,   0(   5) * ret
*         |
  35:    LDA    6,   0(   6) * SP += 0
*       |
  36:    LDC    7, 106(   0) * IP = ???
*       =>else
*       If:
*         =>cond
*         Op:
  37:    LDA    6,  -1(   6) * push BX
  38:     ST    1,   0(   6) * ...
  39:     LD    0,   1(   5) * var value: n
  40:    LDA    1,   0(   0) * BX = AX
  41:    LDC    0,   1(   0) * num 1
  42:    SUB    0,   1,   0  * test and jmp
  43:    JEQ    0,   2(   7) * ...
  44:    LDC    0,   0(   0) * set false
  45:    LDC    7,  47(   0) * skip set true
  46:    LDC    0,   1(   0) * set true
  47:     LD    1,   0(   6) * pop BX
  48:    LDA    6,   1(   6) * ...
*         |
  49:    JEQ    0,   5(   7)
*         =>then
*         Compoundstmt:
  50:    LDA    6,   0(   6) * SP += 0
*           Return:
  51:    LDC    0,   1(   0) * num 1
  52:     LD    7,   0(   5) * ret
*           |
  53:    LDA    6,   0(   6) * SP += 0
*         |
  54:    LDC    7, 106(   0) * IP = ???
*         =>else
*         Compoundstmt:
  55:    LDA    6,   0(   6) * SP += 0
*           Return:
*             Op:
  56:    LDA    6,  -1(   6) * push BX
  57:     ST    1,   0(   6) * ...
*               Fun: f
  58:    LDA    6,  -1(   6) * push BP
  59:     ST    5,   0(   6) * ...
*                 Op:
  60:    LDA    6,  -1(   6) * push BX
  61:     ST    1,   0(   6) * ...
  62:     LD    0,   1(   5) * var value: n
  63:    LDA    1,   0(   0) * BX = AX
  64:    LDC    0,   1(   0) * num 1
  65:    SUB    0,   1,   0  * sub
  66:     LD    1,   0(   6) * pop BX
  67:    LDA    6,   1(   6) * ...
*                 |
  68:    LDA    6,  -1(   6) * push param 0
  69:     ST    0,   0(   6) * ...
  70:    LDC    0,  75(   0) * push return address
  71:    LDA    6,  -1(   6) * ...
  72:     ST    0,   0(   6) * ...
  73:    LDA    5,   0(   6) * BP = SP
  74:    LDC    7,  18(   0) * call function
  75:    LDA    6,   0(   5) * SP = BP
  76:    LDA    6,   2(   6) * SP += 2
  77:     LD    5,   0(   6) * pop BP
  78:    LDA    6,   1(   6) * ...
*               |
  79:    LDA    1,   0(   0) * BX = AX
*               Fun: f
  80:    LDA    6,  -1(   6) * push BP
  81:     ST    5,   0(   6) * ...
*                 Op:
  82:    LDA    6,  -1(   6) * push BX
  83:     ST    1,   0(   6) * ...
  84:     LD    0,   1(   5) * var value: n
  85:    LDA    1,   0(   0) * BX = AX
  86:    LDC    0,   2(   0) * num 2
  87:    SUB    0,   1,   0  * sub
  88:     LD    1,   0(   6) * pop BX
  89:    LDA    6,   1(   6) * ...
*                 |
  90:    LDA    6,  -1(   6) * push param 0
  91:     ST    0,   0(   6) * ...
  92:    LDC    0,  97(   0) * push return address
  93:    LDA    6,  -1(   6) * ...
  94:     ST    0,   0(   6) * ...
  95:    LDA    5,   0(   6) * BP = SP
  96:    LDC    7,  18(   0) * call function
  97:    LDA    6,   0(   5) * SP = BP
  98:    LDA    6,   2(   6) * SP += 2
  99:     LD    5,   0(   6) * pop BP
 100:    LDA    6,   1(   6) * ...
*               |
 101:    ADD    0,   1,   0  * add
 102:     LD    1,   0(   6) * pop BX
 103:    LDA    6,   1(   6) * ...
*             |
 104:     LD    7,   0(   5) * ret
*           |
 105:    LDA    6,   0(   6) * SP += 0
*         |
*         =>end
*       |
*       =>end
*     |
 106:    LDA    6,   0(   6) * SP += 0
*   |
* 
* FunDef: gcd
*   Compoundstmt:
 107:    LDA    6,   0(   6) * SP += 0
*     If:
*       =>cond
*       Op:
 108:    LDA    6,  -1(   6) * push BX
 109:     ST    1,   0(   6) * ...
 110:     LD    0,   2(   5) * var value: v
 111:    LDA    1,   0(   0) * BX = AX
 112:    LDC    0,   0(   0) * num 0
 113:    SUB    0,   1,   0  * test and jmp
 114:    JEQ    0,   2(   7) * ...
 115:    LDC    0,   0(   0) * set false
 116:    LDC    7, 118(   0) * skip set true
 117:    LDC    0,   1(   0) * set true
 118:     LD    1,   0(   6) * pop BX
 119:    LDA    6,   1(   6) * ...
*       |
 120:    JEQ    0,   3(   7)
*       =>then
*       Return:
 121:     LD    0,   1(   5) * var value: u
 122:     LD    7,   0(   5) * ret
*       |
 123:    LDC    7, 163(   0) * IP = ???
*       =>else
*       Return:
*         Fun: gcd
 124:    LDA    6,  -1(   6) * push BP
 125:     ST    5,   0(   6) * ...
*           Op:
 126:    LDA    6,  -1(   6) * push BX
 127:     ST    1,   0(   6) * ...
 128:     LD    0,   1(   5) * var value: u
 129:    LDA    1,   0(   0) * BX = AX
*             Op:
 130:    LDA    6,  -1(   6) * push BX
 131:     ST    1,   0(   6) * ...
*               Op:
 132:    LDA    6,  -1(   6) * push BX
 133:     ST    1,   0(   6) * ...
 134:     LD    0,   1(   5) * var value: u
 135:    LDA    1,   0(   0) * BX = AX
 136:     LD    0,   2(   5) * var value: v
 137:    DIV    0,   1,   0  * div
 138:     LD    1,   0(   6) * pop BX
 139:    LDA    6,   1(   6) * ...
*               |
 140:    LDA    1,   0(   0) * BX = AX
 141:     LD    0,   2(   5) * var value: v
 142:    MUL    0,   1,   0  * mul
 143:     LD    1,   0(   6) * pop BX
 144:    LDA    6,   1(   6) * ...
*             |
 145:    SUB    0,   1,   0  * sub
 146:     LD    1,   0(   6) * pop BX
 147:    LDA    6,   1(   6) * ...
*           |
 148:    LDA    6,  -1(   6) * push param 1
 149:     ST    0,   0(   6) * ...
 150:     LD    0,   2(   5) * var value: v
 151:    LDA    6,  -1(   6) * push param 0
 152:     ST    0,   0(   6) * ...
 153:    LDC    0, 158(   0) * push return address
 154:    LDA    6,  -1(   6) * ...
 155:     ST    0,   0(   6) * ...
 156:    LDA    5,   0(   6) * BP = SP
 157:    LDC    7, 107(   0) * call function
 158:    LDA    6,   0(   5) * SP = BP
 159:    LDA    6,   3(   6) * SP += 3
 160:     LD    5,   0(   6) * pop BP
 161:    LDA    6,   1(   6) * ...
*         |
 162:     LD    7,   0(   5) * ret
*       |
*       =>end
*     |
 163:    LDA    6,   0(   6) * SP += 0
*   |
* 
* FunDef: showArr
*   Compoundstmt:
 164:    LDA    6,  -1(   6) * SP += -1
*     Assign:
 165:    LDA    6,  -1(   6) * push BX
 166:     ST    1,   0(   6) * ...
 167:    LDA    0,  -1(   5) * var address: i
 168:    LDA    6,  -1(   6) * push AX
 169:     ST    0,   0(   6) * ...
 170:    LDC    0,   0(   0) * num 0
 171:     LD    1,   0(   6) * pop BX
 172:    LDA    6,   1(   6) * ...
 173:     ST    0,   0(   1)
 174:     LD    1,   0(   6) * pop BX
 175:    LDA    6,   1(   6) * ...
*     |
*     While:
*       =>cond
*       Op:
 176:    LDA    6,  -1(   6) * push BX
 177:     ST    1,   0(   6) * ...
 178:     LD    0,  -1(   5) * var value: i
 179:    LDA    1,   0(   0) * BX = AX
 180:     LD    0,   2(   5) * var value: n
 181:    SUB    0,   1,   0  * test and jmp
 182:    JLT    0,   2(   7) * ...
 183:    LDC    0,   0(   0) * set false
 184:    LDC    7, 186(   0) * skip set true
 185:    LDC    0,   1(   0) * set true
 186:     LD    1,   0(   6) * pop BX
 187:    LDA    6,   1(   6) * ...
*       |
 188:    JEQ    0,  42(   7)
*       =>then
*       Compoundstmt:
 189:    LDA    6,   0(   6) * SP += 0
*         Fun: output
 190:    LDA    6,  -1(   6) * push BP
 191:     ST    5,   0(   6) * ...
*           Array: arr
 192:    LDA    6,  -1(   6) * push BX
 193:     ST    1,   0(   6) * ...
 194:     LD    1,   1(   5) * BX = data[BP + (1)]
 195:     LD    0,  -1(   5) * var value: i
 196:    SUB    0,   1,   0  * array address: arr
 197:     LD    1,   0(   6) * pop BX
 198:    LDA    6,   1(   6) * ...
 199:     LD    0,   0(   0) * array value: arr
*           |
 200:    LDA    6,  -1(   6) * push param 0
 201:     ST    0,   0(   6) * ...
 202:    LDC    0, 207(   0) * push return address
 203:    LDA    6,  -1(   6) * ...
 204:     ST    0,   0(   6) * ...
 205:    LDA    5,   0(   6) * BP = SP
 206:    LDC    7,  15(   0) * call function
 207:    LDA    6,   0(   5) * SP = BP
 208:    LDA    6,   2(   6) * SP += 2
 209:     LD    5,   0(   6) * pop BP
 210:    LDA    6,   1(   6) * ...
*         |
*         Assign:
 211:    LDA    6,  -1(   6) * push BX
 212:     ST    1,   0(   6) * ...
 213:    LDA    0,  -1(   5) * var address: i
 214:    LDA    6,  -1(   6) * push AX
 215:     ST    0,   0(   6) * ...
*           Op:
 216:    LDA    6,  -1(   6) * push BX
 217:     ST    1,   0(   6) * ...
 218:     LD    0,  -1(   5) * var value: i
 219:    LDA    1,   0(   0) * BX = AX
 220:    LDC    0,   1(   0) * num 1
 221:    ADD    0,   1,   0  * add
 222:     LD    1,   0(   6) * pop BX
 223:    LDA    6,   1(   6) * ...
*           |
 224:     LD    1,   0(   6) * pop BX
 225:    LDA    6,   1(   6) * ...
 226:     ST    0,   0(   1)
 227:     LD    1,   0(   6) * pop BX
 228:    LDA    6,   1(   6) * ...
*         |
 229:    LDA    6,   0(   6) * SP += 0
*       |
 230:    LDC    7, 176(   0) * loop
*       =>end
*     |
 231:    LDA    6,   1(   6) * SP += 1
*   |
 232:     LD    7,   0(   5) * ret
* 
* FunDef: g
*   Compoundstmt:
 233:    LDA    6,   0(   6) * SP += 0
*     If:
*       =>cond
*       Op:
 234:    LDA    6,  -1(   6) * push BX
 235:     ST    1,   0(   6) * ...
 236:     LD    0,   1(   5) * var value: n
 237:    LDA    1,   0(   0) * BX = AX
 238:    LDC    0,   2(   0) * num 2
 239:    SUB    0,   1,   0  * test and jmp
 240:    JLE    0,   2(   7) * ...
 241:    LDC    0,   0(   0) * set false
 242:    LDC    7, 244(   0) * skip set true
 243:    LDC    0,   1(   0) * set true
 244:     LD    1,   0(   6) * pop BX
 245:    LDA    6,   1(   6) * ...
*       |
 246:    JEQ    0,   3(   7)
*       =>then
*       Return:
 247:    LDC    0,  10(   0) * num 10
 248:     LD    7,   0(   5) * ret
*       |
 249:    LDC    7, 250(   0) * IP = ???
*       =>end
*     |
*     Return:
*       Op:
 250:    LDA    6,  -1(   6) * push BX
 251:     ST    1,   0(   6) * ...
*         Op:
 252:    LDA    6,  -1(   6) * push BX
 253:     ST    1,   0(   6) * ...
 254:     LD    0,   1(   5) * var value: n
 255:    LDA    1,   0(   0) * BX = AX
 256:    LDC    0,   2(   0) * num 2
 257:    ADD    0,   1,   0  * add
 258:     LD    1,   0(   6) * pop BX
 259:    LDA    6,   1(   6) * ...
*         |
 260:    LDA    1,   0(   0) * BX = AX
*         Op:
 261:    LDA    6,  -1(   6) * push BX
 262:     ST    1,   0(   6) * ...
 263:     LD    0,   1(   5) * var value: n
 264:    LDA    1,   0(   0) * BX = AX
 265:    LDC    0,   2(   0) * num 2
 266:    SUB    0,   1,   0  * sub
 267:     LD    1,   0(   6) * pop BX
 268:    LDA    6,   1(   6) * ...
*         |
 269:    DIV    0,   1,   0  * div
 270:     LD    1,   0(   6) * pop BX
 271:    LDA    6,   1(   6) * ...
*       |
 272:     LD    7,   0(   5) * ret
*     |
 273:    LDA    6,   0(   6) * SP += 0
*   |
* 
* FunDef: h
*   Compoundstmt:
 274:    LDA    6,   0(   6) * SP += 0
*     Return:
*       Op:
 275:    LDA    6,  -1(   6) * push BX
 276:     ST    1,   0(   6) * ...
 277:     LD    0,   1(   5) * var value: n
 278:    LDA    1,   0(   0) * BX = AX
 279:    LDC    0,   1(   0) * num 1
 280:    ADD    0,   1,   0  * add
 281:     LD    1,   0(   6) * pop BX
 282:    LDA    6,   1(   6) * ...
*       |
 283:     LD    7,   0(   5) * ret
*     |
 284:    LDA    6,   0(   6) * SP += 0
*   |
* 
* FunDef: main
*   Compoundstmt:
 285:    LDA    6,  -7(   6) * SP += -7
*     Fun: showArr
 286:    LDA    6,  -1(   6) * push BP
 287:     ST    5,   0(   6) * ...
 288:    LDC    0,   5(   0) * num 5
 289:    LDA    6,  -1(   6) * push param 1
 290:     ST    0,   0(   6) * ...
 291:    LDA    0,  -1(   5) * array address: a
 292:    LDA    6,  -1(   6) * push param 0
 293:     ST    0,   0(   6) * ...
 294:    LDC    0, 299(   0) * push return address
 295:    LDA    6,  -1(   6) * ...
 296:     ST    0,   0(   6) * ...
 297:    LDA    5,   0(   6) * BP = SP
 298:    LDC    7, 164(   0) * call function
 299:    LDA    6,   0(   5) * SP = BP
 300:    LDA    6,   3(   6) * SP += 3
 301:     LD    5,   0(   6) * pop BP
 302:    LDA    6,   1(   6) * ...
*     |
*     Assign:
 303:    LDA    6,  -1(   6) * push BX
 304:     ST    1,   0(   6) * ...
 305:    LDA    0,  -7(   5) * var address: c
 306:    LDA    6,  -1(   6) * push AX
 307:     ST    0,   0(   6) * ...
 308:    LDC    0,   0(   0) * num 0
 309:     LD    1,   0(   6) * pop BX
 310:    LDA    6,   1(   6) * ...
 311:     ST    0,   0(   1)
 312:     LD    1,   0(   6) * pop BX
 313:    LDA    6,   1(   6) * ...
*     |
*     While:
*       =>cond
*       Op:
 314:    LDA    6,  -1(   6) * push BX
 315:     ST    1,   0(   6) * ...
 316:     LD    0,  -7(   5) * var value: c
 317:    LDA    1,   0(   0) * BX = AX
 318:    LDC    0,   5(   0) * num 5
 319:    SUB    0,   1,   0  * test and jmp
 320:    JLT    0,   2(   7) * ...
 321:    LDC    0,   0(   0) * set false
 322:    LDC    7, 324(   0) * skip set true
 323:    LDC    0,   1(   0) * set true
 324:     LD    1,   0(   6) * pop BX
 325:    LDA    6,   1(   6) * ...
*       |
 326:    JEQ    0,  65(   7)
*       =>then
*       Compoundstmt:
 327:    LDA    6,   0(   6) * SP += 0
*         Assign:
 328:    LDA    6,  -1(   6) * push BX
 329:     ST    1,   0(   6) * ...
*           Array: a
 330:    LDA    6,  -1(   6) * push BX
 331:     ST    1,   0(   6) * ...
 332:    LDA    1,  -1(   5) * BX = BP + (-1)
 333:     LD    0,  -7(   5) * var value: c
 334:    SUB    0,   1,   0  * array address: a
 335:     LD    1,   0(   6) * pop BX
 336:    LDA    6,   1(   6) * ...
*           |
 337:    LDA    6,  -1(   6) * push AX
 338:     ST    0,   0(   6) * ...
*           Fun: f
 339:    LDA    6,  -1(   6) * push BP
 340:     ST    5,   0(   6) * ...
 341:     LD    0,  -7(   5) * var value: c
 342:    LDA    6,  -1(   6) * push param 0
 343:     ST    0,   0(   6) * ...
 344:    LDC    0, 349(   0) * push return address
 345:    LDA    6,  -1(   6) * ...
 346:     ST    0,   0(   6) * ...
 347:    LDA    5,   0(   6) * BP = SP
 348:    LDC    7,  18(   0) * call function
 349:    LDA    6,   0(   5) * SP = BP
 350:    LDA    6,   2(   6) * SP += 2
 351:     LD    5,   0(   6) * pop BP
 352:    LDA    6,   1(   6) * ...
*           |
 353:     LD    1,   0(   6) * pop BX
 354:    LDA    6,   1(   6) * ...
 355:     ST    0,   0(   1)
 356:     LD    1,   0(   6) * pop BX
 357:    LDA    6,   1(   6) * ...
*         |
*         Assign:
 358:    LDA    6,  -1(   6) * push BX
 359:     ST    1,   0(   6) * ...
 360:    LDA    0,  -7(   5) * var address: c
 361:    LDA    6,  -1(   6) * push AX
 362:     ST    0,   0(   6) * ...
*           Op:
 363:    LDA    6,  -1(   6) * push BX
 364:     ST    1,   0(   6) * ...
 365:     LD    0,  -7(   5) * var value: c
 366:    LDA    1,   0(   0) * BX = AX
 367:    LDC    0,   1(   0) * num 1
 368:    ADD    0,   1,   0  * add
 369:     LD    1,   0(   6) * pop BX
 370:    LDA    6,   1(   6) * ...
*           |
 371:     LD    1,   0(   6) * pop BX
 372:    LDA    6,   1(   6) * ...
 373:     ST    0,   0(   1)
 374:     LD    1,   0(   6) * pop BX
 375:    LDA    6,   1(   6) * ...
*         |
*         Fun: output
 376:    LDA    6,  -1(   6) * push BP
 377:     ST    5,   0(   6) * ...
 378:     LD    0,  -7(   5) * var value: c
 379:    LDA    6,  -1(   6) * push param 0
 380:     ST    0,   0(   6) * ...
 381:    LDC    0, 386(   0) * push return address
 382:    LDA    6,  -1(   6) * ...
 383:     ST    0,   0(   6) * ...
 384:    LDA    5,   0(   6) * BP = SP
 385:    LDC    7,  15(   0) * call function
 386:    LDA    6,   0(   5) * SP = BP
 387:    LDA    6,   2(   6) * SP += 2
 388:     LD    5,   0(   6) * pop BP
 389:    LDA    6,   1(   6) * ...
*         |
 390:    LDA    6,   0(   6) * SP += 0
*       |
 391:    LDC    7, 314(   0) * loop
*       =>end
*     |
*     Fun: output
 392:    LDA    6,  -1(   6) * push BP
 393:     ST    5,   0(   6) * ...
 394:     LD    0,  -7(   5) * var value: c
 395:    LDA    6,  -1(   6) * push param 0
 396:     ST    0,   0(   6) * ...
 397:    LDC    0, 402(   0) * push return address
 398:    LDA    6,  -1(   6) * ...
 399:     ST    0,   0(   6) * ...
 400:    LDA    5,   0(   6) * BP = SP
 401:    LDC    7,  15(   0) * call function
 402:    LDA    6,   0(   5) * SP = BP
 403:    LDA    6,   2(   6) * SP += 2
 404:     LD    5,   0(   6) * pop BP
 405:    LDA    6,   1(   6) * ...
*     |
*     Fun: showArr
 406:    LDA    6,  -1(   6) * push BP
 407:     ST    5,   0(   6) * ...
 408:    LDC    0,   5(   0) * num 5
 409:    LDA    6,  -1(   6) * push param 1
 410:     ST    0,   0(   6) * ...
 411:    LDA    0,  -1(   5) * array address: a
 412:    LDA    6,  -1(   6) * push param 0
 413:     ST    0,   0(   6) * ...
 414:    LDC    0, 419(   0) * push return address
 415:    LDA    6,  -1(   6) * ...
 416:     ST    0,   0(   6) * ...
 417:    LDA    5,   0(   6) * BP = SP
 418:    LDC    7, 164(   0) * call function
 419:    LDA    6,   0(   5) * SP = BP
 420:    LDA    6,   3(   6) * SP += 3
 421:     LD    5,   0(   6) * pop BP
 422:    LDA    6,   1(   6) * ...
*     |
*     While:
*       =>cond
*       Op:
 423:    LDA    6,  -1(   6) * push BX
 424:     ST    1,   0(   6) * ...
 425:     LD    0,  -7(   5) * var value: c
 426:    LDA    1,   0(   0) * BX = AX
 427:    LDC    0,   0(   0) * num 0
 428:    SUB    0,   1,   0  * test and jmp
 429:    JGE    0,   2(   7) * ...
 430:    LDC    0,   0(   0) * set false
 431:    LDC    7, 433(   0) * skip set true
 432:    LDC    0,   1(   0) * set true
 433:     LD    1,   0(   6) * pop BX
 434:    LDA    6,   1(   6) * ...
*       |
 435:    JEQ    0, 156(   7)
*       =>then
*       Compoundstmt:
 436:    LDA    6,   0(   6) * SP += 0
*         Assign:
 437:    LDA    6,  -1(   6) * push BX
 438:     ST    1,   0(   6) * ...
*           Array: a
 439:    LDA    6,  -1(   6) * push BX
 440:     ST    1,   0(   6) * ...
 441:    LDA    1,  -1(   5) * BX = BP + (-1)
 442:     LD    0,  -7(   5) * var value: c
 443:    SUB    0,   1,   0  * array address: a
 444:     LD    1,   0(   6) * pop BX
 445:    LDA    6,   1(   6) * ...
*           |
 446:    LDA    6,  -1(   6) * push AX
 447:     ST    0,   0(   6) * ...
*           Fun: gcd
 448:    LDA    6,  -1(   6) * push BP
 449:     ST    5,   0(   6) * ...
*             Op:
 450:    LDA    6,  -1(   6) * push BX
 451:     ST    1,   0(   6) * ...
*               Fun: g
 452:    LDA    6,  -1(   6) * push BP
 453:     ST    5,   0(   6) * ...
*                 Op:
 454:    LDA    6,  -1(   6) * push BX
 455:     ST    1,   0(   6) * ...
*                   Array: a
 456:    LDA    6,  -1(   6) * push BX
 457:     ST    1,   0(   6) * ...
 458:    LDA    1,  -1(   5) * BX = BP + (-1)
 459:     LD    0,  -7(   5) * var value: c
 460:    SUB    0,   1,   0  * array address: a
 461:     LD    1,   0(   6) * pop BX
 462:    LDA    6,   1(   6) * ...
 463:     LD    0,   0(   0) * array value: a
*                   |
 464:    LDA    1,   0(   0) * BX = AX
*                   Fun: g
 465:    LDA    6,  -1(   6) * push BP
 466:     ST    5,   0(   6) * ...
*                     Op:
 467:    LDA    6,  -1(   6) * push BX
 468:     ST    1,   0(   6) * ...
*                       Array: a
 469:    LDA    6,  -1(   6) * push BX
 470:     ST    1,   0(   6) * ...
 471:    LDA    1,  -1(   5) * BX = BP + (-1)
 472:     LD    0,  -7(   5) * var value: c
 473:    SUB    0,   1,   0  * array address: a
 474:     LD    1,   0(   6) * pop BX
 475:    LDA    6,   1(   6) * ...
 476:     LD    0,   0(   0) * array value: a
*                       |
 477:    LDA    1,   0(   0) * BX = AX
 478:    LDC    0,   2(   0) * num 2
 479:    ADD    0,   1,   0  * add
 480:     LD    1,   0(   6) * pop BX
 481:    LDA    6,   1(   6) * ...
*                     |
 482:    LDA    6,  -1(   6) * push param 0
 483:     ST    0,   0(   6) * ...
 484:    LDC    0, 489(   0) * push return address
 485:    LDA    6,  -1(   6) * ...
 486:     ST    0,   0(   6) * ...
 487:    LDA    5,   0(   6) * BP = SP
 488:    LDC    7, 233(   0) * call function
 489:    LDA    6,   0(   5) * SP = BP
 490:    LDA    6,   2(   6) * SP += 2
 491:     LD    5,   0(   6) * pop BP
 492:    LDA    6,   1(   6) * ...
*                   |
 493:    MUL    0,   1,   0  * mul
 494:     LD    1,   0(   6) * pop BX
 495:    LDA    6,   1(   6) * ...
*                 |
 496:    LDA    6,  -1(   6) * push param 0
 497:     ST    0,   0(   6) * ...
 498:    LDC    0, 503(   0) * push return address
 499:    LDA    6,  -1(   6) * ...
 500:     ST    0,   0(   6) * ...
 501:    LDA    5,   0(   6) * BP = SP
 502:    LDC    7, 233(   0) * call function
 503:    LDA    6,   0(   5) * SP = BP
 504:    LDA    6,   2(   6) * SP += 2
 505:     LD    5,   0(   6) * pop BP
 506:    LDA    6,   1(   6) * ...
*               |
 507:    LDA    1,   0(   0) * BX = AX
*               Op:
 508:    LDA    6,  -1(   6) * push BX
 509:     ST    1,   0(   6) * ...
*                 Array: a
 510:    LDA    6,  -1(   6) * push BX
 511:     ST    1,   0(   6) * ...
 512:    LDA    1,  -1(   5) * BX = BP + (-1)
*                   Op:
 513:    LDA    6,  -1(   6) * push BX
 514:     ST    1,   0(   6) * ...
*                     Fun: h
 515:    LDA    6,  -1(   6) * push BP
 516:     ST    5,   0(   6) * ...
 517:     LD    0,  -7(   5) * var value: c
 518:    LDA    6,  -1(   6) * push param 0
 519:     ST    0,   0(   6) * ...
 520:    LDC    0, 525(   0) * push return address
 521:    LDA    6,  -1(   6) * ...
 522:     ST    0,   0(   6) * ...
 523:    LDA    5,   0(   6) * BP = SP
 524:    LDC    7, 274(   0) * call function
 525:    LDA    6,   0(   5) * SP = BP
 526:    LDA    6,   2(   6) * SP += 2
 527:     LD    5,   0(   6) * pop BP
 528:    LDA    6,   1(   6) * ...
*                     |
 529:    LDA    1,   0(   0) * BX = AX
 530:    LDC    0,   1(   0) * num 1
 531:    SUB    0,   1,   0  * sub
 532:     LD    1,   0(   6) * pop BX
 533:    LDA    6,   1(   6) * ...
*                   |
 534:    SUB    0,   1,   0  * array address: a
 535:     LD    1,   0(   6) * pop BX
 536:    LDA    6,   1(   6) * ...
 537:     LD    0,   0(   0) * array value: a
*                 |
 538:    LDA    1,   0(   0) * BX = AX
 539:    LDC    0,   1(   0) * num 1
 540:    ADD    0,   1,   0  * add
 541:     LD    1,   0(   6) * pop BX
 542:    LDA    6,   1(   6) * ...
*               |
 543:    MUL    0,   1,   0  * mul
 544:     LD    1,   0(   6) * pop BX
 545:    LDA    6,   1(   6) * ...
*             |
 546:    LDA    6,  -1(   6) * push param 1
 547:     ST    0,   0(   6) * ...
*             Array: a
 548:    LDA    6,  -1(   6) * push BX
 549:     ST    1,   0(   6) * ...
 550:    LDA    1,  -1(   5) * BX = BP + (-1)
 551:     LD    0,  -7(   5) * var value: c
 552:    SUB    0,   1,   0  * array address: a
 553:     LD    1,   0(   6) * pop BX
 554:    LDA    6,   1(   6) * ...
 555:     LD    0,   0(   0) * array value: a
*             |
 556:    LDA    6,  -1(   6) * push param 0
 557:     ST    0,   0(   6) * ...
 558:    LDC    0, 563(   0) * push return address
 559:    LDA    6,  -1(   6) * ...
 560:     ST    0,   0(   6) * ...
 561:    LDA    5,   0(   6) * BP = SP
 562:    LDC    7, 107(   0) * call function
 563:    LDA    6,   0(   5) * SP = BP
 564:    LDA    6,   3(   6) * SP += 3
 565:     LD    5,   0(   6) * pop BP
 566:    LDA    6,   1(   6) * ...
*           |
 567:     LD    1,   0(   6) * pop BX
 568:    LDA    6,   1(   6) * ...
 569:     ST    0,   0(   1)
 570:     LD    1,   0(   6) * pop BX
 571:    LDA    6,   1(   6) * ...
*         |
*         Assign:
 572:    LDA    6,  -1(   6) * push BX
 573:     ST    1,   0(   6) * ...
 574:    LDA    0,  -7(   5) * var address: c
 575:    LDA    6,  -1(   6) * push AX
 576:     ST    0,   0(   6) * ...
*           Op:
 577:    LDA    6,  -1(   6) * push BX
 578:     ST    1,   0(   6) * ...
 579:     LD    0,  -7(   5) * var value: c
 580:    LDA    1,   0(   0) * BX = AX
 581:    LDC    0,   1(   0) * num 1
 582:    SUB    0,   1,   0  * sub
 583:     LD    1,   0(   6) * pop BX
 584:    LDA    6,   1(   6) * ...
*           |
 585:     LD    1,   0(   6) * pop BX
 586:    LDA    6,   1(   6) * ...
 587:     ST    0,   0(   1)
 588:     LD    1,   0(   6) * pop BX
 589:    LDA    6,   1(   6) * ...
*         |
 590:    LDA    6,   0(   6) * SP += 0
*       |
 591:    LDC    7, 423(   0) * loop
*       =>end
*     |
*     Fun: showArr
 592:    LDA    6,  -1(   6) * push BP
 593:     ST    5,   0(   6) * ...
 594:    LDC    0,   5(   0) * num 5
 595:    LDA    6,  -1(   6) * push param 1
 596:     ST    0,   0(   6) * ...
 597:    LDA    0,  -1(   5) * array address: a
 598:    LDA    6,  -1(   6) * push param 0
 599:     ST    0,   0(   6) * ...
 600:    LDC    0, 605(   0) * push return address
 601:    LDA    6,  -1(   6) * ...
 602:     ST    0,   0(   6) * ...
 603:    LDA    5,   0(   6) * BP = SP
 604:    LDC    7, 164(   0) * call function
 605:    LDA    6,   0(   5) * SP = BP
 606:    LDA    6,   3(   6) * SP += 3
 607:     LD    5,   0(   6) * pop BP
 608:    LDA    6,   1(   6) * ...
*     |
*     Assign:
 609:    LDA    6,  -1(   6) * push BX
 610:     ST    1,   0(   6) * ...
 611:    LDA    0,  -6(   5) * var address: b
 612:    LDA    6,  -1(   6) * push AX
 613:     ST    0,   0(   6) * ...
*       Fun: input
 614:    LDA    6,  -1(   6) * push BP
 615:     ST    5,   0(   6) * ...
 616:    LDC    0, 621(   0) * push return address
 617:    LDA    6,  -1(   6) * ...
 618:     ST    0,   0(   6) * ...
 619:    LDA    5,   0(   6) * BP = SP
 620:    LDC    7,  13(   0) * call function
 621:    LDA    6,   0(   5) * SP = BP
 622:    LDA    6,   1(   6) * SP += 1
 623:     LD    5,   0(   6) * pop BP
 624:    LDA    6,   1(   6) * ...
*       |
 625:     LD    1,   0(   6) * pop BX
 626:    LDA    6,   1(   6) * ...
 627:     ST    0,   0(   1)
 628:     LD    1,   0(   6) * pop BX
 629:    LDA    6,   1(   6) * ...
*     |
*     If:
*       =>cond
 630:     LD    0,  -6(   5) * var value: b
 631:    JEQ    0, 118(   7)
*       =>then
*       Compoundstmt:
 632:    LDA    6,  -1(   6) * SP += -1
*         Assign:
 633:    LDA    6,  -1(   6) * push BX
 634:     ST    1,   0(   6) * ...
 635:    LDA    0,  -8(   5) * var address: a
 636:    LDA    6,  -1(   6) * push AX
 637:     ST    0,   0(   6) * ...
*           Fun: input
 638:    LDA    6,  -1(   6) * push BP
 639:     ST    5,   0(   6) * ...
 640:    LDC    0, 645(   0) * push return address
 641:    LDA    6,  -1(   6) * ...
 642:     ST    0,   0(   6) * ...
 643:    LDA    5,   0(   6) * BP = SP
 644:    LDC    7,  13(   0) * call function
 645:    LDA    6,   0(   5) * SP = BP
 646:    LDA    6,   1(   6) * SP += 1
 647:     LD    5,   0(   6) * pop BP
 648:    LDA    6,   1(   6) * ...
*           |
 649:     LD    1,   0(   6) * pop BX
 650:    LDA    6,   1(   6) * ...
 651:     ST    0,   0(   1)
 652:     LD    1,   0(   6) * pop BX
 653:    LDA    6,   1(   6) * ...
*         |
*         Fun: output
 654:    LDA    6,  -1(   6) * push BP
 655:     ST    5,   0(   6) * ...
*           Op:
 656:    LDA    6,  -1(   6) * push BX
 657:     ST    1,   0(   6) * ...
 658:     LD    0,  -8(   5) * var value: a
 659:    LDA    1,   0(   0) * BX = AX
 660:     LD    0,  -6(   5) * var value: b
 661:    SUB    0,   1,   0  * test and jmp
 662:    JLT    0,   2(   7) * ...
 663:    LDC    0,   0(   0) * set false
 664:    LDC    7, 666(   0) * skip set true
 665:    LDC    0,   1(   0) * set true
 666:     LD    1,   0(   6) * pop BX
 667:    LDA    6,   1(   6) * ...
*           |
 668:    LDA    6,  -1(   6) * push param 0
 669:     ST    0,   0(   6) * ...
 670:    LDC    0, 675(   0) * push return address
 671:    LDA    6,  -1(   6) * ...
 672:     ST    0,   0(   6) * ...
 673:    LDA    5,   0(   6) * BP = SP
 674:    LDC    7,  15(   0) * call function
 675:    LDA    6,   0(   5) * SP = BP
 676:    LDA    6,   2(   6) * SP += 2
 677:     LD    5,   0(   6) * pop BP
 678:    LDA    6,   1(   6) * ...
*         |
*         Compoundstmt:
 679:    LDA    6,   0(   6) * SP += 0
*           Assign:
 680:    LDA    6,  -1(   6) * push BX
 681:     ST    1,   0(   6) * ...
*             Array: d
 682:    LDC    0,   0(   0) * num 0
 683:    LDA    6,  -1(   6) * push BX
 684:     ST    1,   0(   6) * ...
 685:    LDA    1,   0(   0) * BX = AX
 686:    LDC    0,   4(   0) * AX = 4
 687:    SUB    0,   0,   1  * array address: d
 688:     LD    1,   0(   6) * pop BX
 689:    LDA    6,   1(   6) * ...
*             |
 690:    LDA    6,  -1(   6) * push AX
 691:     ST    0,   0(   6) * ...
 692:    LDC    0,   2(   0) * num 2
 693:     LD    1,   0(   6) * pop BX
 694:    LDA    6,   1(   6) * ...
 695:     ST    0,   0(   1)
 696:     LD    1,   0(   6) * pop BX
 697:    LDA    6,   1(   6) * ...
*           |
*           Assign:
 698:    LDA    6,  -1(   6) * push BX
 699:     ST    1,   0(   6) * ...
*             Array: d
 700:    LDC    0,   4(   0) * num 4
 701:    LDA    6,  -1(   6) * push BX
 702:     ST    1,   0(   6) * ...
 703:    LDA    1,   0(   0) * BX = AX
 704:    LDC    0,   4(   0) * AX = 4
 705:    SUB    0,   0,   1  * array address: d
 706:     LD    1,   0(   6) * pop BX
 707:    LDA    6,   1(   6) * ...
*             |
 708:    LDA    6,  -1(   6) * push AX
 709:     ST    0,   0(   6) * ...
 710:    LDC    0,   9(   0) * num 9
 711:     LD    1,   0(   6) * pop BX
 712:    LDA    6,   1(   6) * ...
 713:     ST    0,   0(   1)
 714:     LD    1,   0(   6) * pop BX
 715:    LDA    6,   1(   6) * ...
*           |
 716:    LDA    6,   0(   6) * SP += 0
*         |
*         Fun: showArr
 717:    LDA    6,  -1(   6) * push BP
 718:     ST    5,   0(   6) * ...
 719:    LDC    0,   5(   0) * num 5
 720:    LDA    6,  -1(   6) * push param 1
 721:     ST    0,   0(   6) * ...
 722:    LDC    0,   4(   0) * array address: d
 723:    LDA    6,  -1(   6) * push param 0
 724:     ST    0,   0(   6) * ...
 725:    LDC    0, 730(   0) * push return address
 726:    LDA    6,  -1(   6) * ...
 727:     ST    0,   0(   6) * ...
 728:    LDA    5,   0(   6) * BP = SP
 729:    LDC    7, 164(   0) * call function
 730:    LDA    6,   0(   5) * SP = BP
 731:    LDA    6,   3(   6) * SP += 3
 732:     LD    5,   0(   6) * pop BP
 733:    LDA    6,   1(   6) * ...
*         |
*         Fun: output
 734:    LDA    6,  -1(   6) * push BP
 735:     ST    5,   0(   6) * ...
 736:    LDC    0,11111(   0) * num 11111
 737:    LDA    6,  -1(   6) * push param 0
 738:     ST    0,   0(   6) * ...
 739:    LDC    0, 744(   0) * push return address
 740:    LDA    6,  -1(   6) * ...
 741:     ST    0,   0(   6) * ...
 742:    LDA    5,   0(   6) * BP = SP
 743:    LDC    7,  15(   0) * call function
 744:    LDA    6,   0(   5) * SP = BP
 745:    LDA    6,   2(   6) * SP += 2
 746:     LD    5,   0(   6) * pop BP
 747:    LDA    6,   1(   6) * ...
*         |
 748:    LDA    6,   1(   6) * SP += 1
*       |
 749:    LDC    7, 766(   0) * IP = ???
*       =>else
*       Compoundstmt:
 750:    LDA    6,   0(   6) * SP += 0
*         Fun: output
 751:    LDA    6,  -1(   6) * push BP
 752:     ST    5,   0(   6) * ...
 753:    LDC    0,22222(   0) * num 22222
 754:    LDA    6,  -1(   6) * push param 0
 755:     ST    0,   0(   6) * ...
 756:    LDC    0, 761(   0) * push return address
 757:    LDA    6,  -1(   6) * ...
 758:     ST    0,   0(   6) * ...
 759:    LDA    5,   0(   6) * BP = SP
 760:    LDC    7,  15(   0) * call function
 761:    LDA    6,   0(   5) * SP = BP
 762:    LDA    6,   2(   6) * SP += 2
 763:     LD    5,   0(   6) * pop BP
 764:    LDA    6,   1(   6) * ...
*         |
 765:    LDA    6,   0(   6) * SP += 0
*       |
*       =>end
*     |
 766:    LDA    6,   7(   6) * SP += 7
*   |
 767:     LD    7,   0(   5) * ret
