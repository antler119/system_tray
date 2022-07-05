import 'dart:convert';
import 'dart:io';

import 'package:flutter/services.dart';
import 'package:path/path.dart' show dirname, joinAll;

class Utils {
  static Future<String?> getIcon(String? assetPath) async {
    if (assetPath == null) {
      return null;
    }

    if (assetPath.isEmpty == true) {
      return '';
    }

    if (Platform.isMacOS) {
      return await base64Image(assetPath);
    }

    return joinAll([
      dirname(Platform.resolvedExecutable),
      'data/flutter_assets',
      assetPath,
    ]);
  }

  static Future<String> base64Image(String iconPath) async {
    ByteData imageData = await rootBundle.load(iconPath);
    return base64Encode(imageData.buffer.asUint8List());
  }
}
