#!/usr/bin/env python3

import subprocess
import tempfile
import shutil
import sys
import re
from pathlib import Path


def run_command(cmd, cwd=None):
	try:
		return subprocess.run(cmd, check=True, capture_output=True, text=True, cwd=cwd)
	except subprocess.CalledProcessError as e:
		print(f"✗ Error {cmd}: {e.stderr}")
		sys.exit(1)


def main(repo_root=None, output_dir=None):
	repo_root = Path.cwd() if repo_root is None else Path(repo_root).resolve()
	output_dir = repo_root if output_dir is None else Path(output_dir).resolve()
	output_dir.mkdir(parents=True, exist_ok=True)
	print(f"Working in repository: {repo_root}")

	# Extract version and optional suffix from core/CMakeLists.txt
	cmake_txt = (repo_root / "core" / "CMakeLists.txt").read_text()
	version = re.search(r'ZXing VERSION "?([0-9]+\.[0-9]+\.[0-9]+)"?', cmake_txt).group(1)
	suffix = re.search(r'ZXING_VERSION_SUFFIX "?([^\)\s\"]*)"?', cmake_txt).group(1)
	ver_tag = f"{version}{suffix}"
	print(f"Project version from CMake: {ver_tag}")

	# Create output filenames with git tag
	output_stem = f"zxing-cpp-{ver_tag}"
	output_tgz = output_dir / f"{output_stem}.tar.gz"
	output_zip = output_dir / f"{output_stem}.zip"

	# Step 1: Create initial tar ball via git archive
	print("\n[1/4] Creating git archive...")
	run_command(["git", "archive", f"--prefix={output_stem}/", f"--output={output_tgz}", "HEAD", repo_root])
	print(f"✓ Created git archive: {output_tgz}")

	# Step 2: Extract tar ball into temporary folder
	print("\n[2/4] Extracting archive to temporary folder...")
	with tempfile.TemporaryDirectory() as temp_dir:
		print(f"✓ Created temporary directory: {temp_dir}")

		run_command(["tar", "-xf", str(output_tgz), "-C", temp_dir])
		print(f"✓ Extracted archive to temporary folder")

		# Step 3: Replace files in core/src/libzint/ with files from zint/backend/
		print("\n[3/4] Replacing files in core/src/libzint/ with zint/backend/ files...")
		dst_dir = Path(temp_dir) / output_stem / "core" / "src" / "libzint"
		src_dir = repo_root / "zint" / "backend"  # Use actual repo directory

		# Find all files/symlinks currently in core/src/libzint/ (including subdirectories)
		libzint_files = list(dst_dir.rglob("*"))
		# Include both regular files and symlinks
		libzint_files = [f for f in libzint_files if f.is_file() or f.is_symlink()]

		replaced_count = 0
		not_found_count = 0

		for libzint_file in libzint_files:
			# Get relative path from core/src/libzint/
			rel_path = libzint_file.relative_to(dst_dir)

			# Source path in zint/backend/
			source_file = src_dir / rel_path

			# Only replace if the file exists in zint/backend/
			if source_file.exists() and source_file.is_file():
				# Remove symlink if it exists
				if libzint_file.is_symlink():
					libzint_file.unlink()
				# Copy the actual file
				shutil.copy2(source_file, libzint_file)
				replaced_count += 1
			else:
				not_found_count += 1
				print(f"  - Skipped (not in zint/backend): {rel_path}")

		print(f"✓ Replaced {replaced_count} files, skipped {not_found_count} files")

		# Step 4: Create tar.gz and zip archives
		print("\n[4/4] Creating archives (tar.gz and zip)...")
		run_command(["tar", "-czf", str(output_tgz), "-C", temp_dir, output_stem])

		if (output_zip.exists()):
			output_zip.unlink()

		run_command(["zip", "-q", "-r", str(output_zip), output_stem], cwd=temp_dir)

	# Cleanup initial tar file
	print(f"\n✅ Done! Source archives created:")
	print(f"   - {output_tgz}")
	print(f"   - {output_zip}")


if __name__ == "__main__":
	repo_root = sys.argv[1] if len(sys.argv) > 1 else None
	output_dir = sys.argv[2] if len(sys.argv) > 2 else None
	main(repo_root=repo_root, output_dir=output_dir)
