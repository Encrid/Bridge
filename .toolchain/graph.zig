// Copyright (C) 2026 Enkrid
//
// Under the GNU Affero General Public License, version 3 only: see license.
const std = @import("std");

pub fn build(b: *std.Build) void {
    const path = b.option([]const u8, "graph", "the graph that build.py writes") orelse @panic("-Dgraph is needed");
    b.dependOnFileContents(.{ .cwd_relative = path });
    const text = std.Io.Dir.cwd().readFileAlloc(b.graph.io, path, b.allocator, .unlimited) catch @panic("the graph is not there");
    const graph = std.json.parseFromSliceLeaky(std.json.Value, b.allocator, text, .{}) catch @panic("the graph is not JSON");
    var made = std.StringHashMap(std.Build.LazyPath).init(b.allocator);
    for (graph.object.get("steps").?.array.items) |step| {
        const name = step.object.get("name").?.string;
        const run = b.addSystemCommand(&.{step.object.get("program").?.string});
        run.setCwd(.{ .cwd_relative = step.object.get("cwd").?.string });
        for (step.object.get("args").?.array.items) |arg| {
            const kind = arg.object.get("kind").?.string;
            const prefix = arg.object.get("prefix").?.string;
            const value = arg.object.get("value").?.string;
            if (std.mem.eql(u8, kind, "text")) {
                run.addArg(value);
            } else if (std.mem.eql(u8, kind, "file")) {
                run.addPrefixedFileArg(prefix, .{ .cwd_relative = value });
            } else if (std.mem.eql(u8, kind, "made")) {
                run.addPrefixedFileArg(prefix, made.get(value).?);
            } else if (std.mem.eql(u8, kind, "folder")) {
                run.addPrefixedDirectoryArg(prefix, made.get(value).?.dirname());
            } else if (std.mem.eql(u8, kind, "output")) {
                made.put(name, run.addPrefixedOutputFileArg(prefix, value)) catch @panic("out of memory");
            } else if (std.mem.eql(u8, kind, "deps")) {
                _ = run.addPrefixedDepFileOutputArg(prefix, value);
            }
        }
        if (step.object.get("install")) |install| {
            b.getInstallStep().dependOn(&b.addInstallFile(made.get(name).?, install.string).step);
        }
    }
}
