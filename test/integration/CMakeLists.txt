# filename should be a string pointing to a MIDI file
# sha256 is the expected hash after rendering raw data
function(add_render_test romset filename sha256)
    if("${romset}" STREQUAL "jv880")
        set(ART_RESET_FLAG "")
        set(ART_NVRAM_FLAG "--nvram" "${NUKED_TEST_JV880_NVRAM}")
    else()
        set(ART_RESET_FLAG "--reset" "gm")
        set(ART_NVRAM_FLAG "")
    endif()

    add_test(
        NAME "Render ${romset} ${filename}"
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test_runner.py
            --render-exe $<TARGET_FILE:nuked-sc55-render>
            --sha256 ${sha256}
            --
            ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
            --rom-directory ${NUKED_TEST_ROMDIR}
            --romset ${romset}
            ${ART_RESET_FLAG}
            ${ART_NVRAM_FLAG}
        COMMAND_EXPAND_LISTS
    )
endfunction()

function(add_render_test_multi_instance romset filename instances sha256)
    add_test(
        NAME "Render ${romset} ${filename} instances=${instances}"
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test_runner.py
            --render-exe $<TARGET_FILE:nuked-sc55-render>
            --sha256 ${sha256}
            --
            ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
            --rom-directory ${NUKED_TEST_ROMDIR}
            --romset ${romset}
            --reset gm
            --instances ${instances}
        COMMAND_EXPAND_LISTS
    )
endfunction()

add_render_test("mk2" "avmidi/01.mid" "d9577413d5523f9826062a547d9cbc8013feb4797fb459dad3a50801115c4ecc")
add_render_test("mk2" "avmidi/02.mid" "b02968423b12391152e95f80615d149c6aa47f788ec11cbe2ef05bd46d68fd2f")
add_render_test("mk2" "avmidi/03.mid" "ba78d3bb21bc9266fb1ec51dc88efde08c23e9c2dc23e59ef929a64cdc9e575d")
add_render_test("mk2" "avmidi/04.mid" "0f1c8f63587fe8335cd88eaa4ceff153e50637eb402eddc3f27b55200d5b0adc")
add_render_test("mk2" "avmidi/05.mid" "432badf755752c21a88d4e531bbbb52dc83bcfaf062d420dfcfd2e7a48a3dc41")
add_render_test("mk2" "avmidi/06.mid" "6bcbcf5da6f342611be307ef1088029dd269adb616cb8a9f9b77a3e7fb02eeda")
add_render_test("mk2" "avmidi/07.mid" "104aaec78cd3b923e809f656ecc30bd4fb9629f4b89627f202e40cd3e04c80ac")
add_render_test("mk2" "avmidi/08.mid" "869a22e6cbaf175342b66725a7d1dee0afdbf5274ed920722bf0b284a4718901")
add_render_test("mk2" "avmidi/09.mid" "c8f75a5ade3842802c3018b5e27ece72aaf7d99b1da826d1083fab43211e3ff4")
add_render_test("mk2" "avmidi/0A.mid" "eb3020eb50e7577a5d66ed2cb658194aae9d409f7c28db296af83667b01a2e51")
add_render_test("mk2" "avmidi/0B.mid" "73be3bea757789734f68af7f9272be473e1f52c65f24498c4a6f61e85994e09a")
add_render_test("mk2" "avmidi/0C.mid" "0eb017145e90b6df25e54f2fa450529eb0912df8e379ed0921fd424119aecace")
add_render_test("mk2" "avmidi/10.mid" "ce42746f85338e8f758dd67bd88e2e2aa8e906816a627619de4398e20c51729d")
add_render_test("mk2" "avmidi/11.mid" "8090778147d9982da620eafeddbdb4d5e2bae18c3481f206c411b51f15c09a65")
add_render_test("mk2" "avmidi/12.mid" "f33f4fbf80e18aa5df333b05751445bb4bbb19f6767e5e50abd9a85ebe94a5cc")
add_render_test("mk2" "avmidi/13.mid" "7702a9bd4d42d93f3c13167c0126a979b04ac3f4b6ed964e83e4416f3b49b2fd")
add_render_test("mk2" "avmidi/15.mid" "46735f3cbee73ef4440a7c50b29681f70ef5097fd9d2db818d12d0f93d2384ec")
add_render_test("mk2" "avmidi/16.mid" "9d43d8c17a9a95ad6291b5c2273c98605aa12a817b12ef35228dd968b79a9b68")
add_render_test("mk2" "avmidi/17.mid" "d3770c323bacd632ff1fb201fcd550208e6013c0db3e36df0cfeaa7dcec77106")
add_render_test("mk2" "avmidi/18.mid" "efebadd64d9d175661cf25569e1a3d129b4258db04dd6b28ebc63d3e92d18480")
add_render_test("mk2" "avmidi/19.mid" "ea74d652bdc78aa6a3295005cfe42f2b20c1adf13eb5b231be85ba2bbaeb0439")
add_render_test("mk2" "avmidi/20.mid" "2a3e63d0bceb17f2dbe6a971c5638d7a2b9fae6b5146bd08942599036ca91dfe")
add_render_test("mk2" "avmidi/21.mid" "cc7d5d716ff13ff76d4176ad1b6e4ef3ba8d4d8e7a02f672e47ba565a875889c")
add_render_test("mk2" "avmidi/22.mid" "d96793a65c5a929ab685d671be21b7481dc60c4b875fb75da6d65fd948db94f9")
add_render_test("mk2" "avmidi/23.mid" "df27be96aedbe779ebaf46e4cc32aba58e99587acd6ac44966a6aff44b2d638a")
add_render_test("mk2" "avmidi/24.mid" "01b3ae021263d0a2c06defc743a7f7a7cb1c0ac195d0e59eac651bd44b7aef32")
add_render_test("mk2" "avmidi/25.mid" "12d4a728f622f8fbadd12035c029ae0dc55b43c4120fc1f387119b3bbdf2b3d8")
add_render_test("mk2" "avmidi/25-2.mid" "93126357b8b8d0fbcdd37192543540502fc627edf7d41c0b38ff2d6e3d0fa39d")
add_render_test("mk2" "avmidi/26.mid" "7c6ffa79459d35a95f13f354d8f73024b0b4155d0fb10c64c8c6ded3d8496497")
add_render_test("mk2" "avmidi/27.mid" "ab0dcc330267bbd5478d4ed3b496ea459309f914229d70e92cf05cd50f42756c")
add_render_test("mk2" "avmidi/28.mid" "46e3b8392c0524013af78c2297528f3f44c7833d3f4323a2a9b88ca14f733bfb")
add_render_test("mk2" "avmidi/29.mid" "1e06f2ac4f687596cb1fe515fa5f2d618433d7645d3f7ce78dffc195f6c2f218")
add_render_test("mk2" "avmidi/30.mid" "803c047e8e6bf51db39cf747d916b9a542fab038915dd50245351b9364246869")
add_render_test("mk2" "avmidi/31.mid" "594fdf042c2b6541c3c598ca906af9e0994113dbc2e2669f161b54404f678178")
add_render_test("mk2" "avmidi/32.mid" "040d737a793d1bd15a3081cd77be88076e94b722383bca8a7b229039d378543f")

add_render_test("mk1" "avmidi/01.mid" "b2f280b62dadfba104e3c33072d1a0afbfd3a8b025fbdd006b5c517146824ccf")
add_render_test("mk1" "avmidi/02.mid" "5022ff3a784f26ef51ddf89ad776cafe254d527065e1e6787781a35132e0813a")
add_render_test("mk1" "avmidi/03.mid" "df3ed4d7d2c84842a16550b174311b8c137697b6d246799076445c38097f14bc")
add_render_test("mk1" "avmidi/04.mid" "e71f5bc5f414794175e81d68a93f0c53afa35e980b94908f453a1e49bc9251d2")
add_render_test("mk1" "avmidi/05.mid" "d093b4d65a051983c41bc31bc18d60bae667d4f85efe13faf13c424d2740aa36")
add_render_test("mk1" "avmidi/06.mid" "00cc56ada84cadab14c5887e6b0154621472a85da3fa4e73dca919f4bc55b731")
add_render_test("mk1" "avmidi/07.mid" "c6388a4b1e3d18b0f1d3e04e874dfe755080b3244f02ef6807d12a5bf01448ac")
add_render_test("mk1" "avmidi/08.mid" "01d695309f57d9366ab02c7b0965b94b7dc1ff399be99c303201b537682f100c")
add_render_test("mk1" "avmidi/09.mid" "579dadc4ca70d6f7851e304f27f27131ac878e749ae55d677073826e3d9bd2d5")
add_render_test("mk1" "avmidi/0A.mid" "f701ebef4cfdc8e8fcf5b3b659966968c05931211bd4e4bac86e79540970c114")
add_render_test("mk1" "avmidi/0B.mid" "bd881cda8902449aacb8cf5e3ee71bc9e85eed472bf521b4c192f4c9f666da4b")
add_render_test("mk1" "avmidi/0C.mid" "44c8b4f046de97dead3581845bb16987a2e533f62ae622f37878a189c5fdf2a7")
add_render_test("mk1" "avmidi/10.mid" "b4a82896026c1b126235dc71ebf1d5c168dd236464f2ec21ab924d8a5a7db7b5")
add_render_test("mk1" "avmidi/11.mid" "9b8ae9a15c1c338e28f4b5f552ec9ae90bd482e2dfaeda3e65c8aed96b658fae")
add_render_test("mk1" "avmidi/12.mid" "46228f6166b48400c193569f1a5eafd1e82aded7017269c60f567a287f4048da")
add_render_test("mk1" "avmidi/13.mid" "f23943cec939f4443d0d12e0917d392e4231b98a0a70d836d997054425d69c61")
add_render_test("mk1" "avmidi/15.mid" "e64832b85ac3e0c16372a9198d46ff894e7eaccbc478412e3d889aee64bea632")
add_render_test("mk1" "avmidi/16.mid" "ae9930ce0ed65eb95e461770d90c59d97029ee1048eb3b4fcaa886a87e41e227")
add_render_test("mk1" "avmidi/17.mid" "ee1607cf5e7eb71fc09faec0abfb513652ceb5a9e53fa48c325f276a3780a546")
add_render_test("mk1" "avmidi/18.mid" "fc15d7fe15c1c3470fbd479fbb5f803bbf20c422f8bedbb3c658240ac2e26ce6")
add_render_test("mk1" "avmidi/19.mid" "2fafa8e14f8573c09db35b4490c1daef7b6473850727a2d4e2d6292aabf086dc")
add_render_test("mk1" "avmidi/20.mid" "c41418a172a0b17b9367c338b6a34a94d5262bd34791add29324d54b95ede67d")
add_render_test("mk1" "avmidi/21.mid" "21d9260a9becd635d5bdfaf2a1ddc0c4c98a4d85ef637f1b8bf856cef1b991f9")
add_render_test("mk1" "avmidi/22.mid" "c07fe8c16be38ae4ff3ef4166d20c3d63109acd6ade5b56380001d442a9da0ca")
add_render_test("mk1" "avmidi/23.mid" "00aa8e57ca81493116e59c1b314516f161295732c24794fc1245a6f436296296")
add_render_test("mk1" "avmidi/24.mid" "7b015ebc08f7e9e0c43dce95400178da48f57fdfec48977e862612aaf598cdb7")
add_render_test("mk1" "avmidi/25.mid" "e3c8abc0545d54a30f1435be61c2dee6a84c45e93b3ee0955f6c08efc04a3761")
add_render_test("mk1" "avmidi/25-2.mid" "b86615097972a71d6d1477e5463cea29f79b055198ed80225ac514bbca534aaf")
add_render_test("mk1" "avmidi/26.mid" "eb7b030463c582f6e50ed9e59bcd0abab803a918ed5e31deb00fbe163f154847")
add_render_test("mk1" "avmidi/27.mid" "23734c825c88e09ca5e0d35977a87658ccb9a858fbed78de1ca4bf198b55689c")
add_render_test("mk1" "avmidi/28.mid" "ca64558a71a92c31e16eeb64b23947bebfd6d50f378c9f92fe4ccb436875e0ba")
add_render_test("mk1" "avmidi/29.mid" "b90e7063fc4586e1b3ff205ff54010e64c45d6ba695a25472e0d0699f17f32fe")
add_render_test("mk1" "avmidi/30.mid" "e1087499ea016fbdc150f3fb5311913b7a77104b712c4fdcb7f0b07a11d5e876")
add_render_test("mk1" "avmidi/31.mid" "645c73fb6a086a446e68beafc2afd2026b7eabcf8f3705f68a159f9f60be8f8c")
add_render_test("mk1" "avmidi/32.mid" "d04559f84dc491cdfbe383f89e3b1cf14c4abb292fcb261fdbf486601a36aa86")

add_render_test("mk2" "issue_18/issue_18.mid" "4453907c5db6a35024e20d76909867a5396a069a83d204e65f785adc59897414")

add_render_test_multi_instance("mk2" "issue_42/anacrusis.mid" 2 "8db9e6e53d0d1d070919492638d942e24932387020dfb55be873cf78e2c8bdd5")

add_render_test("jv880" "jv880/jv880.mid" "0cf004b3a568262bdb0c212e2a8af1103129c2deb6eb0275ef9cc2fa946bd968")
