# filename should be a string pointing to a MIDI file
# sha256 is the expected hash after rendering raw data
function(add_render_test romset filename sha256)
    add_test(
        NAME "Render ${romset} ${filename}"
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test_runner.py
            --render-exe $<TARGET_FILE:nuked-sc55-render>
            --sha256 ${sha256}
            --
            ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
            --rom-directory ${NUKED_TEST_ROMDIR}
            --romset ${romset}
        COMMAND_EXPAND_LISTS
    )
endfunction()

add_render_test("mk2" "avmidi/01.mid" "959f4fe0d4f876f57b385bd7c292248451a5f952764506950c8f89118a7dddee")
add_render_test("mk2" "avmidi/02.mid" "acb66c7665c5033c16737a921dfd7d477e1435895c5634113ea3185c71c31f50")
add_render_test("mk2" "avmidi/03.mid" "0c7ce7cbe5831f7221e8448ff941218bab04040fb3426a6ea1d22c9af0e079b6")
add_render_test("mk2" "avmidi/04.mid" "e98bccf3f9ce7bb228402284411837d74bf222d6194bf88d39410c7a8cf29600")
add_render_test("mk2" "avmidi/05.mid" "843c595c63cf05abc4380fb245cd458829c140f4b416b73bf33c942554b6091f")
add_render_test("mk2" "avmidi/06.mid" "6bf009520c4b8a88b853ffb51d262316f07e8a5ef9b5998bdf5e321e12611c5f")
add_render_test("mk2" "avmidi/07.mid" "7958423b9c9e9c7b7eb0dd3f119d5fa29dfe5c0dd6d5a537bf059abf74d32b54")
add_render_test("mk2" "avmidi/08.mid" "1066209eda6e63bcf4bd9c488eb5dfcec493db1d818f122bf441bc26c4e488cc")
add_render_test("mk2" "avmidi/09.mid" "d2dc884a2e6ecee8d1a680340f0e3cf95cd4d2530bd2e6e078589e223874e8ec")
add_render_test("mk2" "avmidi/0A.mid" "6208536e1514203eee647adc8afa433a9989a6b064369e8d07ce77da3ac0694d")
add_render_test("mk2" "avmidi/0B.mid" "ef2c69af9b678e48cc0c6539cfebfaff0487474824e55c049f62c971ca4b1bc5")
add_render_test("mk2" "avmidi/0C.mid" "dd5610f80314b8886eae51d90f4d1bab0bf94ac6cb486f4894c7cb0511eb7b4d")
add_render_test("mk2" "avmidi/10.mid" "65e85e9fbe2e85962642c73b89e0e5fb3a8b9db82b38a306f732fc54ab25173f")
add_render_test("mk2" "avmidi/11.mid" "74efd0e7e4bf14c989a000dc145636701f17377c3fb6a8b6befaddc0d57ec766")
add_render_test("mk2" "avmidi/12.mid" "6f16150a86f20a9fa8e75cdc2acd1227967fab88c7ccc41acc585505399778de")
add_render_test("mk2" "avmidi/13.mid" "139e00db8a68b2a0ec72c71b3b6652e9edb5d57fbb8b577ec1c5b87ab77cd722")
add_render_test("mk2" "avmidi/15.mid" "e8428f668a7345a4d7a223d512763a0414de01a67fe94c81bef352928fef78ef")
add_render_test("mk2" "avmidi/16.mid" "e5f96f3e16e55ad27a8cdc63b6e5c04d17fa96a9f3b03321259619997572790f")
add_render_test("mk2" "avmidi/17.mid" "6355e43e5162cc12d054075406bb240cede5afd198a5474f5f5eab89deb5eddb")
add_render_test("mk2" "avmidi/18.mid" "626e37d8a1d578f6bcb194685157f2b908a57f44d5064aef8f51938a30d0f988")
add_render_test("mk2" "avmidi/19.mid" "bff63fb9f09c2c58acfd5afacad1c9a9583da68965fef840db5f708239e39c5d")
add_render_test("mk2" "avmidi/20.mid" "4fbf60ac138a49e26961962d2aa7f6aa9f9b0aa9ba1d1bde5e38ce93065cb4e0")
add_render_test("mk2" "avmidi/21.mid" "193d0c6d968281ca33149d6fb2ebd4024efef6838d343daf23d05bb45adb914e")
add_render_test("mk2" "avmidi/22.mid" "93da74241b7a9d475aec5eadbf800de62a04cc05259e4b4739d46c86a5952cdc")
add_render_test("mk2" "avmidi/23.mid" "a52635bc4faf77a65b24ff810922a6bb6b03ab873dbf12873167bdddbfc03774")
add_render_test("mk2" "avmidi/24.mid" "66cc848bbcf4822f0b19ca6fcd4f66e8d0f266052b06808ba674920cc206ebcf")
add_render_test("mk2" "avmidi/25.mid" "8a70813b13f5096b9fe30ef6f99fd7ba299020091aa7ccb1f21a8bcf1f094552")
add_render_test("mk2" "avmidi/25-2.mid" "760a3aa92caeaac296dc593a71da0bab7f10e71c2c5f23c61d718c574c147a6f")
add_render_test("mk2" "avmidi/26.mid" "dad4c70ebb82a93fb816c75c9b3e8b47fd8d61a96e400c897fbe5385e371b3a5")
add_render_test("mk2" "avmidi/27.mid" "225abc3e538d88e67cc900de68ed496ef23f5f9107a3b915ec6a882b2b622e3e")
add_render_test("mk2" "avmidi/28.mid" "d734b40deabd074490ce3cff0d71c2b968015e6de38d55d81f488f1c8b04f006")
add_render_test("mk2" "avmidi/29.mid" "11a27ee373ede9ba7718d43540767bdabc14169cd2d38839e23f69d73aea36c7")
add_render_test("mk2" "avmidi/30.mid" "77537ed8177ee627ad1a0c7aa49bbee5d412d24b7e8f3cfc48a505e5d3057dbc")
add_render_test("mk2" "avmidi/31.mid" "1289e7466c67cae745e6b52ff9b7dc60c8ebaef5cdd5b1d483188a404c1365dc")
add_render_test("mk2" "avmidi/32.mid" "2e6fa3195083b9178376315361ad40d76afc5a1adac7557c36c32d6a209c0074")

add_render_test("mk1" "avmidi/01.mid" "bd472ca86b47eb8c34ed318818d7e6fbbff9b3419b885d2c1e7c9980a23b0ef7")
add_render_test("mk1" "avmidi/02.mid" "03f187da1524de6c6c7386ae67d01077f225974cf50e896abaa9c86532ae4c32")
add_render_test("mk1" "avmidi/03.mid" "85a9803ec69b7503eb07ad96ef75d2e3992e02be093dc39bd75a973fc557c074")
add_render_test("mk1" "avmidi/04.mid" "81c5ce081ca94fa22083aa5d9b65d92efb483c6ffb636b7f1a5f1f193d82814e")
add_render_test("mk1" "avmidi/05.mid" "a229a1ecf4e1f7d9bd3ec366a8b8d3f7b7aab353fc8b34e3b12df24513d71350")
add_render_test("mk1" "avmidi/06.mid" "43e34efcd01a6935e9015afba7212b072c82d13d7a327d06b05040b971274e74")
add_render_test("mk1" "avmidi/07.mid" "06402e09ffeffebf6b2f3ac290ac461b62bbbcad5ee3010f31894e4800d62c0f")
add_render_test("mk1" "avmidi/08.mid" "bf208063043c7ca4a3efbff7387e74d7e3664fe60ed9ace5c7c7e9a576f21504")
add_render_test("mk1" "avmidi/09.mid" "509b01352dd2a2c1817fe19dbceab301ddf665d13d9a690dd6410d3b1e3343a9")
add_render_test("mk1" "avmidi/0A.mid" "42ead87b6bf8821070f5228c35d8ad2096da99e9c9a55bcf0e620d2e5601640b")
add_render_test("mk1" "avmidi/0B.mid" "2077fe72200a10a14b66023b58ece34a0ea4d13875d0ad70286affa14fb03dd2")
add_render_test("mk1" "avmidi/0C.mid" "6af1127d8b0d7f795508fd3c06b723cfe5eef8ab01432bee5d2f273ebb3e7a15")
add_render_test("mk1" "avmidi/10.mid" "ecc581dc8ca3930c32f0667c22030cfdee24f1412a564072c502551b9ca955d3")
add_render_test("mk1" "avmidi/11.mid" "61456def32eae76cc924792f8fada2a81b08eff37a258808fe0ec1cd76046a66")
add_render_test("mk1" "avmidi/12.mid" "8f46233ffc94e4f5cb39fa574308e1749bea9bbc6ed7ad767784daa8130b709b")
add_render_test("mk1" "avmidi/13.mid" "1b36dd204dde70be83484378dd41a863f85d787b9b5ec8d9793bac0af0eef6f8")
add_render_test("mk1" "avmidi/15.mid" "9af3c3889ca96718422e62e1772436a0e0cce39292414da28df111389c09617a")
add_render_test("mk1" "avmidi/16.mid" "d8395121565715b584d2a08e662a3208415da113f55a86e6dc848cf4fb0c6225")
add_render_test("mk1" "avmidi/17.mid" "f071ca8f3f064a324155950d507ff4bc4b7a33853762004a95f93d24821f919e")
add_render_test("mk1" "avmidi/18.mid" "4bdf3996e3e8b99e9d8342f3a4ca70bd8879d3ab4e0f78b562e2fc4169c1470d")
add_render_test("mk1" "avmidi/19.mid" "dc32b8c417885f41cc0690c80256934da16dcbe76abff2d55dd6cdc867eec119")
add_render_test("mk1" "avmidi/20.mid" "b6b976afd219f170119f8bd1db1ba9d30f5ff5be97a3e777b513a232ad65aecd")
add_render_test("mk1" "avmidi/21.mid" "c9b38f92e1b4a6b65aaa2ad342bc37206a33aea582a19394dd2f8936101a402c")
add_render_test("mk1" "avmidi/22.mid" "47ad058481f8591bd4ee81c0370dc9d7cd8fcd09daeea7270ca608ccd70ed2aa")
add_render_test("mk1" "avmidi/23.mid" "bcdd9c6679532df065d007a451faf58a6cb202bdb9ca546e6736085f48b4b246")
add_render_test("mk1" "avmidi/24.mid" "e32bba6c97f3d65494178ba1f63a043e82ee0ada5fa8e8a4b09b416542939b85")
add_render_test("mk1" "avmidi/25.mid" "163f724b71b2bcc69e1d5ab76721433e7be7407fb24ddac047418660b442773a")
add_render_test("mk1" "avmidi/25-2.mid" "d58d33771cd5bbd1e112b436792f49dc2aa22d9e887b6c7e8b730f355a4f08a0")
add_render_test("mk1" "avmidi/26.mid" "63e70ff031b8aff3d6bad636b24275c08bf4382012a1264029319eeab442d09e")
add_render_test("mk1" "avmidi/27.mid" "ca23e208173c7c5976b852548157881d5454f0ba3c10d6bad8c5644316b56dd7")
add_render_test("mk1" "avmidi/28.mid" "9cc845cd841ddc8a201b3da18f83c663d61fd9531c3525777680fcec777317d8")
add_render_test("mk1" "avmidi/29.mid" "c2ddcdec3722cb59396e836fdb6469979d4b0f58b2278f2bba5454c920e7a7e1")
add_render_test("mk1" "avmidi/30.mid" "a5766389bdb3536ae0b1ca411194459198be988126e5a15e3db30f424b654297")
add_render_test("mk1" "avmidi/31.mid" "5e0e8310bc90f29286bd554b6a655a9471f790cf8e1c6736bac597ee1ec2e9ad")
add_render_test("mk1" "avmidi/32.mid" "d59c14eaa3d3362979fabba5948005a79aa6d05df5f45bfcc6158433f6ec74df")

add_render_test("mk2" "issue_18/issue_18.mid" "61c0ef6490e0f43d34d9f93583e4255ef897800cd8f34b3f156c14890a58ae10")
