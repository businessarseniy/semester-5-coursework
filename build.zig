const std = @import("std");
const Build = std.Build;
const OptimizeMode = std.builtin.OptimizeMode;
const FeatureSet = std.Target.Cpu.Feature.Set;
const x86Feature = std.Target.x86.Feature;

pub fn build(b: *Build) anyerror!void {
    var enabled_features = FeatureSet.empty;
    var disabled_features = FeatureSet.empty;

    enabled_features.addFeature(@intFromEnum(x86Feature.soft_float));
    disabled_features.addFeature(@intFromEnum(x86Feature.avx));
    disabled_features.addFeature(@intFromEnum(x86Feature.avx2));
    disabled_features.addFeature(@intFromEnum(x86Feature.mmx));
    disabled_features.addFeature(@intFromEnum(x86Feature.sse));
    disabled_features.addFeature(@intFromEnum(x86Feature.sse2));

    const optimize = OptimizeMode.ReleaseSafe;
    const target = b.resolveTargetQuery(.{
        .abi = .none,
        .cpu_arch = .x86,
        .os_tag = .freestanding,
        .cpu_features_add = enabled_features,
        .cpu_features_sub = disabled_features,
    });

    const kernel = b.addExecutable(.{
        .name = "kernel",
        .root_module = b.createModule(.{
            .optimize = optimize,
            .target = target,
            .unwind_tables = .none,
            .pic = true,
        }),
    });

    kernel.addCSourceFiles(.{
        .root = b.path("./kernel/arch/x86/"),
        .files = &.{
            "boot.s",
            "gdt.s",
            "idt.s",
            "isr.s",
            "ring3.s",
        },
    });
    kernel.addCSourceFiles(.{
        .root = b.path("./kernel/filesystem/"),
        .files = &.{
            "vfs.c",
        },
    });
    kernel.addCSourceFiles(.{
        .root = b.path("./kernel/hardware/"),
        .files = &.{
            "hal.c",
            "keyboard.c",
            "pic.c",
            "pit.c",
        },
    });
    kernel.addCSourceFiles(.{
        .root = b.path("./kernel/interrupt/"),
        .files = &.{
            "exception.c",
            "idt.c",
            "syscall.c",
        },
    });
    kernel.addCSourceFiles(.{
        .root = b.path("./kernel/memory/"),
        .files = &.{
            "gdt.c",
            "heap.c",
            "paging.c",
            "physical.c",
            "tss.c",
            "virtual.c",
        },
    });
    kernel.addCSourceFiles(.{
        .root = b.path("./kernel/multitasking/"),
        .files = &.{
            "scheduler.c",
        },
    });
    kernel.addCSourceFiles(.{
        .root = b.path("./kernel/"),
        .files = &.{
            "io.c",
            "kernel.c",
            "queue.c",
            "tty.c",
        },
    });

    kernel.addIncludePath(b.path("include/"));
    kernel.setLinkerScript(b.path("linker.ld"));

    const kernel_step = b.step("kernel", "Build and install kernel executable");
    const kernel_install = b.addInstallArtifact(kernel, .{});
    kernel_step.dependOn(&kernel_install.step);

    const run_step = b.step("run", "Run kernel in emulator");
    const run_cmd = b.addSystemCommand(&.{"qemu-system-i386"});
    run_cmd.addArg("-kernel");
    run_cmd.addArtifactArg(kernel);
    // run_cmd.addArg("-no-reboot");
    // run_cmd.addArg("-no-shutdown");
    run_step.dependOn(kernel_step);
    run_step.dependOn(&run_cmd.step);
}
