echo "clear..."
find ./ -type d -name 'build' -exec echo "Delete {}" \; -exec rm -rf {} +
find ./ -type d -name 'obj' -exec echo "Delete {}" \; -exec rm -rf {} +
find ./ -type d -name '__pycache__' -exec echo "Delete {}" \; -exec rm -rf {} +
find ./ -name '*.o' -exec echo "Delete {}" \; -exec rm -rf {} +
find ./ -name 'app_debug' -exec echo "Delete {}" \; -exec rm -rf {} + 
echo "finish..."