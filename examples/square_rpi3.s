offset: 524288,

mrs [value: mpidr_el1, destination: x1],
and [destination: x1, left: x1, right: 3],
cbz [to: main, test: x1],
haltLoop:
wfe,
b[to: haltLoop],

main:

mov [destination: x10, value: MBOX_CH_PROP],
bl [to: callMailboxW10],

mov [destination: x0, value: 0],
mov [destination: x1, value: 0],
mov [destination: x2, value: 0],
mov [destination: x3, value: 0],
mov [destination: x4, value: 0],
mov [destination: x5, value: 0],
mov [destination: x6, value: 0],
mov [destination: x7, value: 0],

adr [destination: x0, value: mbox],

ldr [valueAt: x0, offset: 112, destination: w1],
and [left: 1073741823, right: w1, destination: w1],

ldr [valueAt: x0, offset: 132, destination: w2],

ldr [valueAt: x0, offset: 20, destination: w3],

mov [value: 4, destination: x4],
mul [left: x3, right: x4, destination: x3],

ldr [valueAt: x0, offset: 24, destination: w4],

mov [destination: x0, value: 0],
mov [destination: x9, value: 0],

pixelLoopAll:
mov [destination: x8, value: 0],

mov [value: x9, destination: x0],
add [left: x9, right: 8, destination: x9],

pixelLoopRow:
mov [destination: x7, value: 0],

mov [value: x0, destination: x10],
mov [destination: x11, value: 2],
udiv [destination: x10, left: x10, right: x11],
bl [to: callIntToColorX10],
add [destination: x0, left: x0, right: 1],

pixelLoopCol:
mul [left: x2, right: x8, destination: x6],
add [left: x6, right: x7, destination: x6],
add [left: x1, right: x6, destination: x6],
str [destinationAt: x6, value: w10],
add [left: x7, right: 4, destination: x7],
cmp [left: x7, right: x3],
b [cond: lt, to: pixelLoopCol],

add [left: x8, right: 1, destination: x8],
cmp [left: x8, right: x4],
b [cond: lt, to: pixelLoopRow],

b [to: pixelLoopAll],


terminateLoop: b [to: terminateLoop],

callIntToColorX10:
mov [destination: x11, value: 768],
udiv [destination: x12, left: x10, right: x11],
msub [destination: x10, left: x12, right: x11, subFrom: x10],
cmp [left: x10, right: 256],
b [cond: lt, to: lessThan256],
cmp [left: x10, right: 512],
b [cond: lt, to: lessThan512],
b [to: lessThan768],
lessThan256:
    mvn [destination: x11, value: x10],
    and [left: x11, right: 255, destination: x11],
    
    lsl [value: w11, destination: w11, shift: 16],
    orr [left: x10, right: x11, destination: x10],
ret,
lessThan512:
    sub [left: x10, right: 256, destination: x10],
    
    mvn [destination: x11, value: x10],
    and [left: x11, right: 255, destination: x11],
    
    lsl [value: x10, destination: x10, shift: 8],
    orr [left: x10, right: x11, destination: x10],
ret,
lessThan768:
    sub [left: x10, right: 512, destination: x10],
    
    mvn [destination: x11, value: x10],
    and [left: x11, right: 255, destination: x11],
    
    lsl [value: x10, destination: x10, shift: 16],
    lsl [value: x11, destination: x11, shift: 8],
    orr [left: x10, right: x11, destination: x10],
ret,

callMailboxW10:
ldr [valueAt: MBOX_STATUS, destination: x0],
ldr [valueAt: x0, destination: w9],
nop,
and [left: w9, right: MBOX_FULL, destination: w9],
cbnz [test: w9, to: callMailboxW10],

adr [destination: x9, value: mbox],
and [destination: w9, left: w9, right: -16],
and [destination: w10, left: w10, right: 15],
orr [destination: w10, left: w9, right: w10],

ldr [valueAt: MBOX_WRITE, destination: x0],
str [destinationAt: x0, value: w10],

listenForMailboxSuccess:
ldr [valueAt: MBOX_STATUS, destination: x0],
ldr [valueAt: x0, destination: w9],
nop,
and [left: w9, right: MBOX_EMPTY, destination: w9],
cbnz [test: w9, to: listenForMailboxSuccess],

ldr [valueAt: MBOX_READ, destination: x0],
ldr [valueAt: x0, destination: w9],
cmp [left: w9, right: w10],
b [cond: ne, to: listenForMailboxSuccess],


adr [destination: x0, value: mbox],
ldr [valueAt: x0, offset: 4, destination: w1],
mov [value: MBOX_RESPONSE, destination: w0],
mov [value: 0, destination: x10],
sub [left: w1, right: w0, destination: w10],
ret,

align [to: 8],
MBOX_REQUEST: 0,

MBOX_CH_POWER: 0,
MBOX_CH_FB: 1,
MBOX_CH_VUART: 2,
MBOX_CH_VCHIQ: 3,
MBOX_CH_LEDS: 4,
MBOX_CH_BTNS: 5,
MBOX_CH_TOUCH: 6,
MBOX_CH_COUNT: 7,
MBOX_CH_PROP: 8,

MBOX_TAG_SETPOWER: 163841,
MBOX_TAG_SETCLKRATE: 38002,
MBOX_TAG_LAST: 0,

MMIO_BASE: dword 1056964608,

VIDEOCORE_MBOX: dword 1057011840,
MBOX_READ: dword 1057011840,
MBOX_POLL: dword 1057011856,
MBOX_SENDER: dword 1057011860,
MBOX_STATUS: dword 1057011864,
MBOX_CONFIG: dword 1057011868,
MBOX_WRITE: dword 1057011872,

MBOX_RESPONSE  : 2147483648,
MBOX_FULL      : 2147483648,
MBOX_EMPTY     : 1073741824,

align [to: 16],
mbox: word [
    140,
    MBOX_REQUEST,

    294915,
    8,
    8,
    1920,
    1080,

    294916,
    8,
    8,
    1920,
    1080,

    294921,
    8,
    8,
    0,
    0,

    294917,
    4,4,32,

    294918,
    4,4,1,

    262145,
    8,8,4096,0,

    262152,4,4,0,
    MBOX_TAG_LAST,
],
