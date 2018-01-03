.global _isr_kb
_isr_kb:
  cli
  call key_handler
  call send_EOI
  sti
  iretq

.global _isr_timer1
_isr_timer1:
#  call send_EOI
  cli
  pushq %rax
  pushq %rbx
  pushq %rcx
  pushq %rdx
  pushq %rbp
  pushq %rsi
  pushq %rdi
  pushq %r8
  pushq %r9
  pushq %r10
  pushq %r11
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15

  call print_timer
  call send_EOI
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %r11
  popq %r10
  popq %r9
  popq %r8
  popq %rdi
  popq %rsi
  popq %rbp
  popq %rdx
  popq %rcx
  popq %rbx
  popq %rax

  sti
  iretq

.global _isr_sys_def0
_isr_sys_def0:
  call send_EOI
  iretq

.global _isr_sys_def1
_isr_sys_def1:
  call send_EOI
  iretq

.global _isr_sys_def2
_isr_sys_def2:
  call send_EOI
  iretq

.global _isr_sys_def3
_isr_sys_def3:
  call send_EOI
  iretq

.global _isr_sys_def4
_isr_sys_def4:
  call send_EOI
  iretq

.global _isr_sys_def5
_isr_sys_def5:
  call send_EOI
  iretq

.global _isr_sys_def6
_isr_sys_def6:
  call send_EOI
  iretq

.global _isr_sys_def7
_isr_sys_def7:
  call send_EOI
  iretq

.global _isr_sys_def8
_isr_sys_def8:
  call send_EOI
  iretq

.global _isr_sys_def9
_isr_sys_def9:
  call send_EOI
  iretq

.global _isr_sys_def10
_isr_sys_def10:
  call send_EOI
  iretq

.global _isr_sys_def11
_isr_sys_def11:
  call send_EOI
  iretq

.global _isr_sys_def12
_isr_sys_def12:
  call send_EOI
  iretq

.global _isr_sys_def13
_isr_sys_def13:
  call send_EOI
  iretq

.global _isr_sys_def14
_isr_sys_def14:
  cli
  pushq %rax
  pushq %rbx
  pushq %rcx
  pushq %rdx
  pushq %rbp
  pushq %rsi
  pushq %rdi
  pushq %r8
  pushq %r9
  pushq %r10
  pushq %r11
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
  movq 120(%rsp), %rdi
  call page_fault_handler
#  call send_EOI
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %r11
  popq %r10
  popq %r9
  popq %r8
  popq %rdi
  popq %rsi
  popq %rbp
  popq %rdx
  popq %rcx
  popq %rbx
  popq %rax
  add $8, %rsp
  sti
  iretq

.global _isr_sys_def15
_isr_sys_def15:
  call send_EOI
  iretq

.global _isr_sys_def16
_isr_sys_def16:
  call send_EOI
  iretq

.global _isr_sys_def17
_isr_sys_def17:
  call send_EOI
  iretq

.global _isr_sys_def18
_isr_sys_def18:
  call send_EOI
  iretq

.global _isr_sys_def19
_isr_sys_def19:
  call send_EOI
  iretq

.global _isr_sys_def20
_isr_sys_def20:
  call send_EOI
  iretq

.global _isr_sys_def21
_isr_sys_def21:
  call send_EOI
  iretq

.global _isr_sys_def22
_isr_sys_def22:
  call send_EOI
  iretq

.global _isr_sys_def23
_isr_sys_def23:
  call send_EOI
  iretq

.global _isr_sys_def24
_isr_sys_def24:
  call send_EOI
  iretq

.global _isr_sys_def25
_isr_sys_def25:
  call send_EOI
  iretq

.global _isr_sys_def26
_isr_sys_def26:
  call send_EOI
  iretq

.global _isr_sys_def27
_isr_sys_def27:
  call send_EOI
  iretq

.global _isr_sys_def28
_isr_sys_def28:
  call send_EOI
  iretq

.global _isr_sys_def29
_isr_sys_def29:
  call send_EOI
  iretq

.global _isr_sys_def30
_isr_sys_def30:
  call send_EOI
  iretq

.global _isr_sys_def31
_isr_sys_def31:
  call send_EOI
  iretq



.global _isr_sys_def34
 _isr_sys_def34:
 iretq


.global _isr_sys_def35
 _isr_sys_def35:
 iretq


.global _isr_sys_def36
 _isr_sys_def36:
 iretq


.global _isr_sys_def37
 _isr_sys_def37:
 iretq


.global _isr_sys_def38
 _isr_sys_def38:
 iretq


.global _isr_sys_def39
 _isr_sys_def39:
 iretq


.global _isr_sys_def40
 _isr_sys_def40:
 iretq


.global _isr_sys_def41
 _isr_sys_def41:
 iretq


.global _isr_sys_def42
 _isr_sys_def42:
 iretq


.global _isr_sys_def43
 _isr_sys_def43:
 iretq


.global _isr_sys_def44
 _isr_sys_def44:
 iretq


.global _isr_sys_def45
 _isr_sys_def45:
 iretq


.global _isr_sys_def46
 _isr_sys_def46:
 iretq


.global _isr_sys_def47
 _isr_sys_def47:
 iretq


.global _isr_sys_def48
 _isr_sys_def48:
 iretq


.global _isr_sys_def49
 _isr_sys_def49:
 iretq


.global _isr_sys_def50
 _isr_sys_def50:
 iretq


.global _isr_sys_def51
 _isr_sys_def51:
 iretq


.global _isr_sys_def52
 _isr_sys_def52:
 iretq


.global _isr_sys_def53
 _isr_sys_def53:
 iretq


.global _isr_sys_def54
 _isr_sys_def54:
 iretq


.global _isr_sys_def55
 _isr_sys_def55:
 iretq


.global _isr_sys_def56
 _isr_sys_def56:
 iretq


.global _isr_sys_def57
 _isr_sys_def57:
 iretq


.global _isr_sys_def58
 _isr_sys_def58:
 iretq


.global _isr_sys_def59
 _isr_sys_def59:
 iretq


.global _isr_sys_def60
 _isr_sys_def60:
 iretq


.global _isr_sys_def61
 _isr_sys_def61:
 iretq


.global _isr_sys_def62
 _isr_sys_def62:
 iretq


.global _isr_sys_def63
 _isr_sys_def63:
 iretq


.global _isr_sys_def64
 _isr_sys_def64:
 iretq


.global _isr_sys_def65
 _isr_sys_def65:
 iretq


.global _isr_sys_def66
 _isr_sys_def66:
 iretq


.global _isr_sys_def67
 _isr_sys_def67:
 iretq


.global _isr_sys_def68
 _isr_sys_def68:
 iretq


.global _isr_sys_def69
 _isr_sys_def69:
 iretq


.global _isr_sys_def70
 _isr_sys_def70:
 iretq


.global _isr_sys_def71
 _isr_sys_def71:
 iretq


.global _isr_sys_def72
 _isr_sys_def72:
 iretq


.global _isr_sys_def73
 _isr_sys_def73:
 iretq


.global _isr_sys_def74
 _isr_sys_def74:
 iretq


.global _isr_sys_def75
 _isr_sys_def75:
 iretq


.global _isr_sys_def76
 _isr_sys_def76:
 iretq


.global _isr_sys_def77
 _isr_sys_def77:
 iretq


.global _isr_sys_def78
 _isr_sys_def78:
 iretq


.global _isr_sys_def79
 _isr_sys_def79:
 iretq


.global _isr_sys_def80
 _isr_sys_def80:
 iretq


.global _isr_sys_def81
 _isr_sys_def81:
 iretq


.global _isr_sys_def82
 _isr_sys_def82:
 iretq


.global _isr_sys_def83
 _isr_sys_def83:
 iretq


.global _isr_sys_def84
 _isr_sys_def84:
 iretq


.global _isr_sys_def85
 _isr_sys_def85:
 iretq


.global _isr_sys_def86
 _isr_sys_def86:
 iretq


.global _isr_sys_def87
 _isr_sys_def87:
 iretq


.global _isr_sys_def88
 _isr_sys_def88:
 iretq


.global _isr_sys_def89
 _isr_sys_def89:
 iretq


.global _isr_sys_def90
 _isr_sys_def90:
 iretq


.global _isr_sys_def91
 _isr_sys_def91:
 iretq


.global _isr_sys_def92
 _isr_sys_def92:
 iretq


.global _isr_sys_def93
 _isr_sys_def93:
 iretq


.global _isr_sys_def94
 _isr_sys_def94:
 iretq


.global _isr_sys_def95
 _isr_sys_def95:
 iretq


.global _isr_sys_def96
 _isr_sys_def96:
 iretq


.global _isr_sys_def97
 _isr_sys_def97:
 iretq


.global _isr_sys_def98
 _isr_sys_def98:
 iretq


.global _isr_sys_def99
 _isr_sys_def99:
 iretq


.global _isr_sys_def100
 _isr_sys_def100:
 iretq


.global _isr_sys_def101
 _isr_sys_def101:
 iretq


.global _isr_sys_def102
 _isr_sys_def102:
 iretq


.global _isr_sys_def103
 _isr_sys_def103:
 iretq


.global _isr_sys_def104
 _isr_sys_def104:
 iretq


.global _isr_sys_def105
 _isr_sys_def105:
 iretq


.global _isr_sys_def106
 _isr_sys_def106:
 iretq


.global _isr_sys_def107
 _isr_sys_def107:
 iretq


.global _isr_sys_def108
 _isr_sys_def108:
 iretq


.global _isr_sys_def109
 _isr_sys_def109:
 iretq


.global _isr_sys_def110
 _isr_sys_def110:
 iretq


.global _isr_sys_def111
 _isr_sys_def111:
 iretq


.global _isr_sys_def112
 _isr_sys_def112:
 iretq


.global _isr_sys_def113
 _isr_sys_def113:
 iretq


.global _isr_sys_def114
 _isr_sys_def114:
 iretq


.global _isr_sys_def115
 _isr_sys_def115:
 iretq


.global _isr_sys_def116
 _isr_sys_def116:
 iretq


.global _isr_sys_def117
 _isr_sys_def117:
 iretq


.global _isr_sys_def118
 _isr_sys_def118:
 iretq


.global _isr_sys_def119
 _isr_sys_def119:
 iretq


.global _isr_sys_def120
 _isr_sys_def120:
 iretq


.global _isr_sys_def121
 _isr_sys_def121:
 iretq


.global _isr_sys_def122
 _isr_sys_def122:
 iretq


.global _isr_sys_def123
 _isr_sys_def123:
 iretq


.global _isr_sys_def124
 _isr_sys_def124:
 iretq


.global _isr_sys_def125
 _isr_sys_def125:
 iretq


.global _isr_sys_def126
 _isr_sys_def126:
 iretq


.global _isr_sys_def127
 _isr_sys_def127:
 iretq


.global _isr_sys_def128
 _isr_sys_def128:
 iretq


.global _isr_sys_def129
 _isr_sys_def129:
 iretq


.global _isr_sys_def130
 _isr_sys_def130:
 iretq


.global _isr_sys_def131
 _isr_sys_def131:
 iretq


.global _isr_sys_def132
 _isr_sys_def132:
 iretq


.global _isr_sys_def133
 _isr_sys_def133:
 iretq


.global _isr_sys_def134
 _isr_sys_def134:
 iretq


.global _isr_sys_def135
 _isr_sys_def135:
 iretq


.global _isr_sys_def136
 _isr_sys_def136:
 iretq


.global _isr_sys_def137
 _isr_sys_def137:
 iretq


.global _isr_sys_def138
 _isr_sys_def138:
 iretq


.global _isr_sys_def139
 _isr_sys_def139:
 iretq


.global _isr_sys_def140
 _isr_sys_def140:
 iretq


.global _isr_sys_def141
 _isr_sys_def141:
 iretq


.global _isr_sys_def142
 _isr_sys_def142:
 iretq


.global _isr_sys_def143
 _isr_sys_def143:
 iretq


.global _isr_sys_def144
 _isr_sys_def144:
 iretq


.global _isr_sys_def145
 _isr_sys_def145:
 iretq


.global _isr_sys_def146
 _isr_sys_def146:
 iretq


.global _isr_sys_def147
 _isr_sys_def147:
 iretq


.global _isr_sys_def148
 _isr_sys_def148:
 iretq


.global _isr_sys_def149
 _isr_sys_def149:
 iretq


.global _isr_sys_def150
 _isr_sys_def150:
 iretq


.global _isr_sys_def151
 _isr_sys_def151:
 iretq


.global _isr_sys_def152
 _isr_sys_def152:
 iretq


.global _isr_sys_def153
 _isr_sys_def153:
 iretq


.global _isr_sys_def154
 _isr_sys_def154:
 iretq


.global _isr_sys_def155
 _isr_sys_def155:
 iretq


.global _isr_sys_def156
 _isr_sys_def156:
 iretq


.global _isr_sys_def157
 _isr_sys_def157:
 iretq


.global _isr_sys_def158
 _isr_sys_def158:
 iretq


.global _isr_sys_def159
 _isr_sys_def159:
 iretq


.global _isr_sys_def160
 _isr_sys_def160:
 iretq


.global _isr_sys_def161
 _isr_sys_def161:
 iretq


.global _isr_sys_def162
 _isr_sys_def162:
 iretq


.global _isr_sys_def163
 _isr_sys_def163:
 iretq


.global _isr_sys_def164
 _isr_sys_def164:
 iretq


.global _isr_sys_def165
 _isr_sys_def165:
 iretq


.global _isr_sys_def166
 _isr_sys_def166:
 iretq


.global _isr_sys_def167
 _isr_sys_def167:
 iretq


.global _isr_sys_def168
 _isr_sys_def168:
 iretq


.global _isr_sys_def169
 _isr_sys_def169:
 iretq


.global _isr_sys_def170
 _isr_sys_def170:
 iretq


.global _isr_sys_def171
 _isr_sys_def171:
 iretq


.global _isr_sys_def172
 _isr_sys_def172:
 iretq


.global _isr_sys_def173
 _isr_sys_def173:
 iretq


.global _isr_sys_def174
 _isr_sys_def174:
 iretq


.global _isr_sys_def175
 _isr_sys_def175:
 iretq


.global _isr_sys_def176
 _isr_sys_def176:
 iretq


.global _isr_sys_def177
 _isr_sys_def177:
 iretq


.global _isr_sys_def178
 _isr_sys_def178:
 iretq


.global _isr_sys_def179
 _isr_sys_def179:
 iretq


.global _isr_sys_def180
 _isr_sys_def180:
 iretq


.global _isr_sys_def181
 _isr_sys_def181:
 iretq


.global _isr_sys_def182
 _isr_sys_def182:
 iretq


.global _isr_sys_def183
 _isr_sys_def183:
 iretq


.global _isr_sys_def184
 _isr_sys_def184:
 iretq


.global _isr_sys_def185
 _isr_sys_def185:
 iretq


.global _isr_sys_def186
 _isr_sys_def186:
 iretq


.global _isr_sys_def187
 _isr_sys_def187:
 iretq


.global _isr_sys_def188
 _isr_sys_def188:
 iretq


.global _isr_sys_def189
 _isr_sys_def189:
 iretq


.global _isr_sys_def190
 _isr_sys_def190:
 iretq


.global _isr_sys_def191
 _isr_sys_def191:
 iretq


.global _isr_sys_def192
 _isr_sys_def192:
 iretq


.global _isr_sys_def193
 _isr_sys_def193:
 iretq


.global _isr_sys_def194
 _isr_sys_def194:
 iretq


.global _isr_sys_def195
 _isr_sys_def195:
 iretq


.global _isr_sys_def196
 _isr_sys_def196:
 iretq


.global _isr_sys_def197
 _isr_sys_def197:
 iretq


.global _isr_sys_def198
 _isr_sys_def198:
 iretq


.global _isr_sys_def199
 _isr_sys_def199:
 iretq


.global _isr_sys_def200
 _isr_sys_def200:
 iretq


.global _isr_sys_def201
 _isr_sys_def201:
 iretq


.global _isr_sys_def202
 _isr_sys_def202:
 iretq


.global _isr_sys_def203
 _isr_sys_def203:
 iretq


.global _isr_sys_def204
 _isr_sys_def204:
 iretq


.global _isr_sys_def205
 _isr_sys_def205:
 iretq


.global _isr_sys_def206
 _isr_sys_def206:
 iretq


.global _isr_sys_def207
 _isr_sys_def207:
 iretq


.global _isr_sys_def208
 _isr_sys_def208:
 iretq


.global _isr_sys_def209
 _isr_sys_def209:
 iretq


.global _isr_sys_def210
 _isr_sys_def210:
 iretq


.global _isr_sys_def211
 _isr_sys_def211:
 iretq


.global _isr_sys_def212
 _isr_sys_def212:
 iretq


.global _isr_sys_def213
 _isr_sys_def213:
 iretq


.global _isr_sys_def214
 _isr_sys_def214:
 iretq


.global _isr_sys_def215
 _isr_sys_def215:
 iretq


.global _isr_sys_def216
 _isr_sys_def216:
 iretq


.global _isr_sys_def217
 _isr_sys_def217:
 iretq


.global _isr_sys_def218
 _isr_sys_def218:
 iretq


.global _isr_sys_def219
 _isr_sys_def219:
 iretq


.global _isr_sys_def220
 _isr_sys_def220:
 iretq


.global _isr_sys_def221
 _isr_sys_def221:
 iretq


.global _isr_sys_def222
 _isr_sys_def222:
 iretq


.global _isr_sys_def223
 _isr_sys_def223:
 iretq


.global _isr_sys_def224
 _isr_sys_def224:
 iretq


.global _isr_sys_def225
 _isr_sys_def225:
 iretq


.global _isr_sys_def226
 _isr_sys_def226:
 iretq


.global _isr_sys_def227
 _isr_sys_def227:
 iretq


.global _isr_sys_def228
 _isr_sys_def228:
 iretq


.global _isr_sys_def229
 _isr_sys_def229:
 iretq


.global _isr_sys_def230
 _isr_sys_def230:
 iretq


.global _isr_sys_def231
 _isr_sys_def231:
 iretq


.global _isr_sys_def232
 _isr_sys_def232:
 iretq


.global _isr_sys_def233
 _isr_sys_def233:
 iretq


.global _isr_sys_def234
 _isr_sys_def234:
 iretq


.global _isr_sys_def235
 _isr_sys_def235:
 iretq


.global _isr_sys_def236
 _isr_sys_def236:
 iretq


.global _isr_sys_def237
 _isr_sys_def237:
 iretq


.global _isr_sys_def238
 _isr_sys_def238:
 iretq


.global _isr_sys_def239
 _isr_sys_def239:
 iretq


.global _isr_sys_def240
 _isr_sys_def240:
 iretq


.global _isr_sys_def241
 _isr_sys_def241:
 iretq


.global _isr_sys_def242
 _isr_sys_def242:
 iretq


.global _isr_sys_def243
 _isr_sys_def243:
 iretq


.global _isr_sys_def244
 _isr_sys_def244:
 iretq


.global _isr_sys_def245
 _isr_sys_def245:
 iretq


.global _isr_sys_def246
 _isr_sys_def246:
 iretq


.global _isr_sys_def247
 _isr_sys_def247:
 iretq


.global _isr_sys_def248
 _isr_sys_def248:
 iretq


.global _isr_sys_def249
 _isr_sys_def249:
 iretq


.global _isr_sys_def250
 _isr_sys_def250:
 iretq


.global _isr_sys_def251
 _isr_sys_def251:
 iretq


.global _isr_sys_def252
 _isr_sys_def252:
 iretq


.global _isr_sys_def253
 _isr_sys_def253:
 iretq


.global _isr_sys_def254
 _isr_sys_def254:
 iretq


.global _isr_sys_def255
 _isr_sys_def255:
 iretq
