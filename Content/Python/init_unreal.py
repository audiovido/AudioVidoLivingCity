
# >>> UNREAL_AGENT_AUTO_BRIDGE >>>
# Managed by UnrealAgent_FINAL_ONECLICK_v2.bat
import socket as _ua_socket
import traceback as _ua_traceback
import unreal as _ua_unreal

_ua_listener = r"C:\Users\Shadow\Desktop\Unreal-Agent\tools\unreal\ue_listener.py"
_ua_host = "127.0.0.1"
_ua_port = 6766
_ua_tick_handle = None

def _ua_port_open():
    _s = _ua_socket.socket(_ua_socket.AF_INET, _ua_socket.SOCK_STREAM)
    _s.settimeout(0.20)
    try:
        return _s.connect_ex((_ua_host, _ua_port)) == 0
    finally:
        _s.close()

def _ua_start_bridge(_delta=0.0):
    global _ua_tick_handle

    if _ua_tick_handle is not None:
        try:
            _ua_unreal.unregister_slate_post_tick_callback(_ua_tick_handle)
        except Exception:
            pass
        _ua_tick_handle = None

    if _ua_port_open():
        _ua_unreal.log("[UnrealAgent] Bridge already listening on 127.0.0.1:6766")
        return

    try:
        with open(_ua_listener, "r", encoding="utf-8-sig") as _f:
            _src = _f.read()

        _ns = {"__name__": "__main__", "__file__": _ua_listener}
        exec(compile(_src, _ua_listener, "exec"), _ns, _ns)
        _ua_unreal.log("[UnrealAgent] Automatic bridge startup completed.")
    except Exception:
        _ua_unreal.log_error(
            "[UnrealAgent] Automatic bridge startup failed:\n" +
            _ua_traceback.format_exc()
        )

try:
    _ua_tick_handle = _ua_unreal.register_slate_post_tick_callback(_ua_start_bridge)
except Exception:
    _ua_start_bridge(0.0)
# <<< UNREAL_AGENT_AUTO_BRIDGE <<<
