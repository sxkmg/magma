"""
Copyright 2020 The Magma Authors.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

from lte.protos.mconfig import mconfigs_pb2
from magma.enodebd.devices.device_utils import EnodebDeviceName


class EnodebConfigBuilder:
    @classmethod
    def get_mconfig(
        cls,
        device: EnodebDeviceName = EnodebDeviceName.BAICELLS,
    ) -> mconfigs_pb2.EnodebD:
        mconfig = mconfigs_pb2.EnodebD()
        mconfig.bandwidth_mhz = 20
        # This earfcndl is actually unused, remove later
        mconfig.earfcndl = 44490
        mconfig.log_level = 1
        mconfig.plmnid_list = "00101"
        mconfig.pci = 260
        mconfig.allow_enodeb_transmit = False
        mconfig.tac = 1
        if device is EnodebDeviceName.BAICELLS_QAFB:
            # fdd config
            mconfig.fdd_config.earfcndl = 9211
        elif device is EnodebDeviceName.CAVIUM:
            # fdd config
            mconfig.fdd_config.earfcndl = 2405
        elif device is EnodebDeviceName.BAICELLS_RTS:
            device_id = '120200002618AGP0003'

            mconfig.enb_configs_by_serial[device_id] \
                .bandwidth_mhz = 20
            mconfig.enb_configs_by_serial[device_id] \
                .cell_id = 138777000
            mconfig.enb_configs_by_serial[device_id] \
                .device_class = "Baicells Nova-243 OD TDD"
            mconfig.enb_configs_by_serial[device_id] \
                .earfcndl = 44390
            mconfig.enb_configs_by_serial[device_id] \
                .pci = 260
            mconfig.enb_configs_by_serial[device_id] \
                .special_subframe_pattern = 7
            mconfig.enb_configs_by_serial[device_id] \
                .subframe_assignment = 2
            mconfig.enb_configs_by_serial[device_id] \
                .tac = 1
            mconfig.enb_configs_by_serial[device_id] \
                .transmit_enabled = False
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.a1_threshold_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.a2_threshold_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.a3_offset = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.a3_offset_anr = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.a4_threshold_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.lte_intra_a5_threshold_1_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.lte_intra_a5_threshold_2_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.lte_inter_anr_a5_threshold_1_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.lte_inter_anr_a5_threshold_2_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.b2_threshold1_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.b2_threshold2_rsrp = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.b2_geran_irat_threshold = 8
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.qrxlevmin_sib1 = -23
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.qrxlevmin_sib3 = -23
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.qrxlevminoffset = 8
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.s_intrasearch = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.s_nonintrasearch = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.reselection_priority = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.threshservinglow = 0
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.ciphering_algorithm = "128-EEA1,128-EEA2"
            mconfig.enb_configs_by_serial[device_id] \
                .ho_algorithm_config.integrity_algorithm = "128-EIA1,128-EIA2"
        else:
            # tdd config
            mconfig.tdd_config.earfcndl = 39150
            mconfig.tdd_config.subframe_assignment = 2
            mconfig.tdd_config.special_subframe_pattern = 7

        return mconfig

    @classmethod
    def get_multi_enb_mconfig(
        cls,
    ) -> mconfigs_pb2.EnodebD:
        mconfig = mconfigs_pb2.EnodebD()
        mconfig.bandwidth_mhz = 20
        mconfig.special_subframe_pattern = 7
        # This earfcndl is actually unused, remove later
        mconfig.earfcndl = 44490
        mconfig.log_level = 1
        mconfig.plmnid_list = "00101"
        mconfig.pci = 260
        mconfig.allow_enodeb_transmit = False
        mconfig.subframe_assignment = 2
        mconfig.tac = 1

        # tdd config, unused because of multi-enb config
        mconfig.tdd_config.earfcndl = 39150
        mconfig.tdd_config.subframe_assignment = 2
        mconfig.tdd_config.special_subframe_pattern = 7

        id1 = '120200002618AGP0003'
        #enb_conf_1 = mconfigs_pb2.EnodebD.EnodebConfig()
        mconfig.enb_configs_by_serial[id1]\
                .earfcndl = 39151
        mconfig.enb_configs_by_serial[id1]\
                .subframe_assignment = 2
        mconfig.enb_configs_by_serial[id1]\
                .special_subframe_pattern = 7
        mconfig.enb_configs_by_serial[id1]\
                .pci = 259
        mconfig.enb_configs_by_serial[id1]\
                .bandwidth_mhz = 20
        mconfig.enb_configs_by_serial[id1] \
            .tac = 1
        mconfig.enb_configs_by_serial[id1] \
            .cell_id = 0
        mconfig.enb_configs_by_serial[id1]\
            .transmit_enabled = True
        mconfig.enb_configs_by_serial[id1]\
            .device_class = 'Baicells Band 40'

        id2 = '120200002618AGP0004'
        #enb_conf_2 = mconfigs_pb2.EnodebD.EnodebConfig()
        mconfig.enb_configs_by_serial[id2]\
                .earfcndl = 39151
        mconfig.enb_configs_by_serial[id2]\
                .subframe_assignment = 2
        mconfig.enb_configs_by_serial[id2]\
                .special_subframe_pattern = 7
        mconfig.enb_configs_by_serial[id2]\
                .pci = 261
        mconfig.enb_configs_by_serial[id2] \
            .bandwidth_mhz = 20
        mconfig.enb_configs_by_serial[id2] \
            .tac = 1
        mconfig.enb_configs_by_serial[id2] \
            .cell_id = 0
        mconfig.enb_configs_by_serial[id2]\
                .transmit_enabled = True
        mconfig.enb_configs_by_serial[id2]\
                .device_class = 'Baicells Band 40'

        return mconfig

    @classmethod
    def get_service_config(cls):
        return {
            "tr069": {
                "interface": "eth1",
                "port": 48080,
                "perf_mgmt_port": 8081,
                "public_ip": "192.88.99.142",
            },
            "reboot_enodeb_on_mme_disconnected": True,
            "s1_interface": "eth1",
        }
