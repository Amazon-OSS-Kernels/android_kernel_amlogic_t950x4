#! /bin/bash

export CROSS_COMPILE=/opt/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

make ARCH=arm64 gxm_q200_2g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxm_q201_2g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxm_skt.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxl_p212_2g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxl_p212_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxl_sei210_2g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxl_sei210_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxl_p400_2g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxl_p401_2g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 gxl_skt.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 axg_pxp.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 axg_a113d_skt.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 axg_s400.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 axg_s420.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 axg_s400_v03.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 axg_s420_v03.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 g12a_pxp.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 sm1_pxp.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 g12a_s905d2_skt.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 g12b_pxp.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 g12b_a311d_skt.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 g12b_a311d_w400.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 tm2_revb_pxp.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 tm2_t962e2_ab311.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 tm2_t962e2_ab319.dtb || echo "Compile dtb Fail!!"

make ARCH=arm64 tm2_revb_t962e2_ab311.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 tm2_revb_t962e2_ab319.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 tm2_revb_t962x3_ab301.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 tm2_revb_t962x3_ab309.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 tm2_revb_t962x3_t312.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 tm2_v901d_t501.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5_pxp.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5_t963_ak301.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5_t963_ak309.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5_t963_ak329.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_pxp.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950d4_am309_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950d4_am301_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950x4_am319_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950x4_am311_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950d4_am309_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950d4_am301_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950x4_am319_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950x4_am311_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950d4_am309_1.5g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_t950d4_am301_1.5g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_pxp.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950d4_am309_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950d4_am301_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950x4_am319_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950x4_am311_1g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950d4_am309_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950d4_am301_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950x4_am319_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950x4_am311_512m.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950d4_am309_1.5g.dtb || echo "Compile dtb Fail !!"

make ARCH=arm64 t5d_reva_t950d4_am301_1.5g.dtb || echo "Compile dtb Fail !!"
