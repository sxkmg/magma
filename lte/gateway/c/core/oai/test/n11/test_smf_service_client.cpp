/**
 * Copyright 2020 The Magma Authors.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include <glog/logging.h>
#include "lte/gateway/c/core/oai/include/mme_config.h"

#include "lte/protos/session_manager.pb.h"
#include "lte/gateway/c/core/oai/lib/n11/SmfServiceClient.h"

using ::testing::Test;

struct mme_config_s mme_config;

namespace magma {
namespace lte {

TEST(test_create_sm_pdu_session_v4, create_sm_pdu_session_v4) {
  SetSMSessionContext request;

  std::string imsi("901700000000001");
  std::string apn("magmacore.com");
  uint32_t pdu_session_id = 0x5;
  uint32_t pdu_session_type = 3;
  uint32_t gnb_gtp_teid = 1;
  uint8_t pti = 10;
  uint8_t gnb_gtp_teid_ip_addr[4] = {0};  //("10.20.30.40")
  gnb_gtp_teid_ip_addr[0] = 0xA;
  gnb_gtp_teid_ip_addr[1] = 0x14;
  gnb_gtp_teid_ip_addr[2] = 0x1E;
  gnb_gtp_teid_ip_addr[3] = 0x28;
  std::string gnb_ip_addr;
  for (int i = 0; i < 4; ++i) {
    gnb_ip_addr += std::to_string(gnb_gtp_teid_ip_addr[i]);
    if (i != 3) {
      gnb_ip_addr += ".";
    }
  }

  std::string ue_ipv4_addr("10.20.30.44");
  std::string ue_ipv6_addr;

  uint32_t version = 0;

  ambr_t default_ambr;

  request = magma5g::create_sm_pdu_session(
      imsi, (uint8_t*)apn.c_str(), pdu_session_id, pdu_session_type,
      gnb_gtp_teid, pti, gnb_gtp_teid_ip_addr, ue_ipv4_addr, ue_ipv6_addr,
      default_ambr, version);

  auto* rat_req =
      request.mutable_rat_specific_context()->mutable_m5gsm_session_context();
  auto* req_cmn = request.mutable_common_context();

  EXPECT_TRUE(imsi == req_cmn->sid().id().substr(4));
  EXPECT_TRUE(magma::lte::SubscriberID_IDType::SubscriberID_IDType_IMSI ==
              req_cmn->sid().type());
  EXPECT_TRUE(apn == req_cmn->apn());
  EXPECT_TRUE(magma::lte::RATType::TGPP_NR == req_cmn->rat_type());
  EXPECT_TRUE(magma::lte::SMSessionFSMState::CREATING_0 ==
              req_cmn->sm_session_state());
  EXPECT_TRUE(0 == req_cmn->sm_session_version());
  EXPECT_TRUE(pdu_session_id == rat_req->pdu_session_id());
  EXPECT_TRUE(magma::lte::RequestType::INITIAL_REQUEST ==
              rat_req->request_type());

  EXPECT_TRUE(magma::lte::PduSessionType::IPV4 == rat_req->pdu_session_type());
  EXPECT_TRUE(1 == rat_req->mutable_gnode_endpoint()->teid());
  EXPECT_TRUE(gnb_ip_addr ==
              rat_req->mutable_gnode_endpoint()->end_ipv4_addr());
  uint8_t* pti_decoded = (uint8_t*)rat_req->procedure_trans_identity().c_str();
  EXPECT_TRUE(pti == *pti_decoded);
  EXPECT_TRUE(ue_ipv4_addr == req_cmn->ue_ipv4());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}  // namespace lte
}  // namespace magma
