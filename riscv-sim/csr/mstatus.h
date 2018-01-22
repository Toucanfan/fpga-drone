#ifndef RVSIM_CSR_MSTATUS_H
#define RVSIM_CSR_MSTATUS_H 1

#define MST_UIE   (1 << 0)  /* Always zero */
#define MST_SIE   (1 << 1)  /* Always zero */
#define MST_MIE   (1 << 3)
#define MST_UPIE  (1 << 4)  /* Always zero */
#define MST_SPIE  (1 << 5)  /* Always zero */
#define MST_MPIE  (1 << 7)
#define MST_SPP   (1 << 8)  /* Always zero */
#define MST_MPP0  (1 << 11) /* Always 1 */
#define MST_MPP1  (1 << 12) /* Always 1 */
#define MST_FS0   (1 << 13) /* Always zero */
#define MST_FS1   (1 << 14) /* Always zero */
#define MST_XS0   (1 << 15) /* Always zero */
#define MST_XS1   (1 << 16) /* Always zero */
#define MST_MPRV  (1 << 17) /* Always zero */
#define MST_SUM   (1 << 18) /* Always zero */
#define MST_MXR   (1 << 19) /* Always zero */
#define MST_TVM   (1 << 20) /* Always zero */
#define MST_TW    (1 << 21) /* Always zero */
#define MST_TSR   (1 << 22) /* Always zero */
#define MST_SD    (1 << 31) /* Always zero */

#endif /* RVSIM_CSR_MSTATUS_H */
