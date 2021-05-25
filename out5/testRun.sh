#!/bin/bash
for filename in ./*.png; do
	echo "$filename" >> test.txt
	image-similarity-measures --org_img_path=square_output.png --pred_img_path="$filename" --metric=ssim >> test.txt
	image-similarity-measures --org_img_path=square_output.png --pred_img_path="$filename" --metric=rmse >> test.txt
	image-similarity-measures --org_img_path=square_output.png --pred_img_path="$filename" --metric=fsim >> test.txt
	image-similarity-measures --org_img_path=square_output.png --pred_img_path="$filename" --metric=psnr >> test.txt
done
