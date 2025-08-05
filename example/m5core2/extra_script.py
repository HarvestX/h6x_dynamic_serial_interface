Import("env")
import os

src_dir = os.path.abspath(os.path.join("..", "..", "h6x_dynamic_packet_handler", "src"))
lib = env.BuildLibrary(
    os.path.join(env.subst("$BUILD_DIR"), "h6x_dynamic_packet_handler_lib"),
    src_dir
)
env.Append(LIBS=[lib])