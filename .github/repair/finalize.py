from pathlib import Path


def replace_once(path: str, old: str, new: str) -> None:
    target = Path(path)
    text = target.read_text()
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected exactly one match, found {count}")
    target.write_text(text.replace(old, new, 1))


replace_once(
    ".github/workflows/ci.yml",
    "          grep -Fq 'LinkStandardObdView(snapshot: obdSnapshot)' app/ios/MBLINK/MBLINKApp.swift\n",
    "          grep -Fq 'MBStandardOBDView()' app/ios/MBLINK/MBLINKApp.swift\n"
    "          grep -Fq 'private struct MBStandardOBDView: View' app/ios/MBLINK/MBLINKApp.swift\n"
    "          ! grep -Fq 'LinkStandardObdView(snapshot: obdSnapshot)' app/ios/MBLINK/MBLINKApp.swift\n",
)

replace_once(
    "tests/test_mercedes_module_scan.c",
    '''        CHECK(strcmp(command, "1902FF") == 0);\n        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==\n              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);\n\n        CHECK(send_ok(&scan, "ATSP6") == 0);\n        CHECK(send_ok(&scan, "ATSH64A") == 0);\n''',
    '''        CHECK(strcmp(command, "1902FF") == 0);\n        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==\n              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);\n        CHECK(scan.stage ==\n              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_DEFAULT_SESSION);\n        CHECK(mblink_mercedes_module_scan_command(\n                  &scan, command, sizeof(command), &written) ==\n              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);\n        CHECK(strcmp(command, "1001") == 0);\n        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==\n              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);\n\n        CHECK(send_ok(&scan, "ATSP6") == 0);\n        CHECK(send_ok(&scan, "ATSH64A") == 0);\n''',
)

replace_once(
    "tests/test_mercedes_module_scan.c",
    '''        CHECK(accept_identity_metadata(\n                  &scan, &no_data, &no_data, &no_data) == 0);\n        CHECK(scan.full_target_index == 1U);\n        CHECK(scan.candidate_tx == UINT32_C(0x632));\n        CHECK(scan.candidate_rx == UINT32_C(0x486));\n''',
    '''        CHECK(accept_identity_metadata(\n                  &scan, &no_data, &no_data, &no_data) == 0);\n        CHECK(scan.stage ==\n              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DEFAULT_SESSION);\n        CHECK(scan.full_target_index == 0U);\n        CHECK(mblink_mercedes_module_scan_command(\n                  &scan, command, sizeof(command), &written) ==\n              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);\n        CHECK(strcmp(command, "1001") == 0);\n        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==\n              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);\n        CHECK(scan.full_target_index == 1U);\n        CHECK(scan.candidate_tx == UINT32_C(0x632));\n        CHECK(scan.candidate_rx == UINT32_C(0x486));\n''',
)
